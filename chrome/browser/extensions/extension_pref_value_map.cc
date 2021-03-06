// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_pref_value_map.h"

#include "base/stl_util.h"
#include "base/values.h"
#include "chrome/browser/prefs/pref_value_map.h"

using extensions::ExtensionPrefsScope;

struct ExtensionPrefValueMap::ExtensionEntry {
  // Installation time of the extension.
  base::Time install_time;
  // Whether extension is enabled in the profile.
  bool enabled;
  // Extension controlled preferences for the regular profile.
  PrefValueMap regular_profile_preferences;
  // Extension controlled preferences that should *only* apply to the regular
  // profile.
  PrefValueMap regular_only_profile_preferences;
  // Persistent extension controlled preferences for the incognito profile,
  // empty for regular profile ExtensionPrefStore.
  PrefValueMap incognito_profile_preferences_persistent;
  // Session only extension controlled preferences for the incognito profile.
  // These preferences are deleted when the incognito profile is destroyed.
  PrefValueMap incognito_profile_preferences_session_only;
};

ExtensionPrefValueMap::ExtensionPrefValueMap() : destroyed_(false) {
}

ExtensionPrefValueMap::~ExtensionPrefValueMap() {
  if (!destroyed_) {
    NotifyOfDestruction();
    destroyed_ = true;
  }
  STLDeleteValues(&entries_);
  entries_.clear();
}

void ExtensionPrefValueMap::Shutdown() {
  NotifyOfDestruction();
  destroyed_ = true;
}

void ExtensionPrefValueMap::SetExtensionPref(const std::string& ext_id,
                                             const std::string& key,
                                             ExtensionPrefsScope scope,
                                             Value* value) {
  PrefValueMap* prefs = GetExtensionPrefValueMap(ext_id, scope);

  if (prefs->SetValue(key, value))
    NotifyPrefValueChanged(key);
}

void ExtensionPrefValueMap::RemoveExtensionPref(
    const std::string& ext_id,
    const std::string& key,
    ExtensionPrefsScope scope) {
  PrefValueMap* prefs = GetExtensionPrefValueMap(ext_id, scope);
  if (prefs->RemoveValue(key))
    NotifyPrefValueChanged(key);
}

bool ExtensionPrefValueMap::CanExtensionControlPref(
    const std::string& extension_id,
    const std::string& pref_key,
    bool incognito) const {
  ExtensionEntryMap::const_iterator ext = entries_.find(extension_id);
  if (ext == entries_.end()) {
    NOTREACHED();
    return false;
  }

  ExtensionEntryMap::const_iterator winner =
      GetEffectivePrefValueController(pref_key, incognito, NULL);
  if (winner == entries_.end())
    return true;

  return winner->second->install_time <= ext->second->install_time;
}

void ExtensionPrefValueMap::ClearAllIncognitoSessionOnlyPreferences() {
  typedef std::set<std::string> KeySet;
  KeySet deleted_keys;

  ExtensionEntryMap::iterator i;
  for (i = entries_.begin(); i != entries_.end(); ++i) {
    PrefValueMap& inc_prefs =
        i->second->incognito_profile_preferences_session_only;
    PrefValueMap::iterator j;
    for (j = inc_prefs.begin(); j != inc_prefs.end(); ++j)
      deleted_keys.insert(j->first);
    inc_prefs.Clear();
  }

  KeySet::iterator k;
  for (k = deleted_keys.begin(); k != deleted_keys.end(); ++k)
    NotifyPrefValueChanged(*k);
}

bool ExtensionPrefValueMap::DoesExtensionControlPref(
    const std::string& extension_id,
    const std::string& pref_key,
    bool* from_incognito) const {
  bool incognito = (from_incognito != NULL);
  ExtensionEntryMap::const_iterator winner =
      GetEffectivePrefValueController(pref_key, incognito, from_incognito);
  if (winner == entries_.end())
    return false;
  return winner->first == extension_id;
}

void ExtensionPrefValueMap::RegisterExtension(const std::string& ext_id,
                                              const base::Time& install_time,
                                              bool is_enabled) {
  if (entries_.find(ext_id) != entries_.end())
    UnregisterExtension(ext_id);
  entries_[ext_id] = new ExtensionEntry;
  entries_[ext_id]->install_time = install_time;
  entries_[ext_id]->enabled = is_enabled;
}

void ExtensionPrefValueMap::UnregisterExtension(const std::string& ext_id) {
  ExtensionEntryMap::iterator i = entries_.find(ext_id);
  if (i == entries_.end())
    return;
  std::set<std::string> keys;  // keys set by this extension
  GetExtensionControlledKeys(*(i->second), &keys);

  delete i->second;
  entries_.erase(i);

  NotifyPrefValueChanged(keys);
}

void ExtensionPrefValueMap::SetExtensionState(const std::string& ext_id,
                                              bool is_enabled) {
  ExtensionEntryMap::const_iterator i = entries_.find(ext_id);
  // This may happen when sync sets the extension state for an
  // extension that is not installed.
  if (i == entries_.end())
    return;
  if (i->second->enabled == is_enabled)
    return;
  std::set<std::string> keys;  // keys set by this extension
  GetExtensionControlledKeys(*(i->second), &keys);
  i->second->enabled = is_enabled;
  NotifyPrefValueChanged(keys);
}

PrefValueMap* ExtensionPrefValueMap::GetExtensionPrefValueMap(
    const std::string& ext_id,
    ExtensionPrefsScope scope) {
  ExtensionEntryMap::const_iterator i = entries_.find(ext_id);
  CHECK(i != entries_.end());
  switch (scope) {
    case extensions::kExtensionPrefsScopeRegular:
      return &(i->second->regular_profile_preferences);
    case extensions::kExtensionPrefsScopeRegularOnly:
      return &(i->second->regular_only_profile_preferences);
    case extensions::kExtensionPrefsScopeIncognitoPersistent:
      return &(i->second->incognito_profile_preferences_persistent);
    case extensions::kExtensionPrefsScopeIncognitoSessionOnly:
      return &(i->second->incognito_profile_preferences_session_only);
  }
  NOTREACHED();
  return NULL;
}

const PrefValueMap* ExtensionPrefValueMap::GetExtensionPrefValueMap(
    const std::string& ext_id,
    ExtensionPrefsScope scope) const {
  ExtensionEntryMap::const_iterator i = entries_.find(ext_id);
  CHECK(i != entries_.end());
  switch (scope) {
    case extensions::kExtensionPrefsScopeRegular:
      return &(i->second->regular_profile_preferences);
    case extensions::kExtensionPrefsScopeRegularOnly:
      return &(i->second->regular_only_profile_preferences);
    case extensions::kExtensionPrefsScopeIncognitoPersistent:
      return &(i->second->incognito_profile_preferences_persistent);
    case extensions::kExtensionPrefsScopeIncognitoSessionOnly:
      return &(i->second->incognito_profile_preferences_session_only);
  }
  NOTREACHED();
  return NULL;
}

void ExtensionPrefValueMap::GetExtensionControlledKeys(
    const ExtensionEntry& entry,
    std::set<std::string>* out) const {
  PrefValueMap::const_iterator i;

  const PrefValueMap& regular_prefs = entry.regular_profile_preferences;
  for (i = regular_prefs.begin(); i != regular_prefs.end(); ++i)
    out->insert(i->first);

  const PrefValueMap& regular_only_prefs =
      entry.regular_only_profile_preferences;
  for (i = regular_only_prefs.begin(); i != regular_only_prefs.end(); ++i)
    out->insert(i->first);

  const PrefValueMap& inc_prefs_pers =
      entry.incognito_profile_preferences_persistent;
  for (i = inc_prefs_pers.begin(); i != inc_prefs_pers.end(); ++i)
    out->insert(i->first);

  const PrefValueMap& inc_prefs_session =
      entry.incognito_profile_preferences_session_only;
  for (i = inc_prefs_session.begin(); i != inc_prefs_session.end(); ++i)
    out->insert(i->first);
}

const Value* ExtensionPrefValueMap::GetEffectivePrefValue(
    const std::string& key,
    bool incognito,
    bool* from_incognito) const {
  ExtensionEntryMap::const_iterator winner =
      GetEffectivePrefValueController(key, incognito, from_incognito);
  if (winner == entries_.end())
    return NULL;

  const Value* value = NULL;
  const std::string& ext_id = winner->first;

  // First search for incognito session only preferences.
  if (incognito) {
    const PrefValueMap* prefs = GetExtensionPrefValueMap(
        ext_id, extensions::kExtensionPrefsScopeIncognitoSessionOnly);
    prefs->GetValue(key, &value);
    if (value)
      return value;

    // If no incognito session only preference exists, fall back to persistent
    // incognito preference.
    prefs = GetExtensionPrefValueMap(
        ext_id,
        extensions::kExtensionPrefsScopeIncognitoPersistent);
    prefs->GetValue(key, &value);
    if (value)
      return value;
  } else {
    // Regular-only preference.
    const PrefValueMap* prefs = GetExtensionPrefValueMap(
        ext_id, extensions::kExtensionPrefsScopeRegularOnly);
    prefs->GetValue(key, &value);
    if (value)
      return value;
  }

  // Regular preference.
  const PrefValueMap* prefs = GetExtensionPrefValueMap(
      ext_id, extensions::kExtensionPrefsScopeRegular);
  prefs->GetValue(key, &value);
  return value;
}

ExtensionPrefValueMap::ExtensionEntryMap::const_iterator
ExtensionPrefValueMap::GetEffectivePrefValueController(
    const std::string& key,
    bool incognito,
    bool* from_incognito) const {
  ExtensionEntryMap::const_iterator winner = entries_.end();
  base::Time winners_install_time;

  ExtensionEntryMap::const_iterator i;
  for (i = entries_.begin(); i != entries_.end(); ++i) {
    const std::string& ext_id = i->first;
    const base::Time& install_time = i->second->install_time;
    const bool enabled = i->second->enabled;

    if (!enabled)
      continue;
    if (install_time < winners_install_time)
      continue;

    const Value* value = NULL;
    const PrefValueMap* prefs = GetExtensionPrefValueMap(
        ext_id, extensions::kExtensionPrefsScopeRegular);
    if (prefs->GetValue(key, &value)) {
      winner = i;
      winners_install_time = install_time;
      if (from_incognito)
        *from_incognito = false;
    }

    if (!incognito) {
      const PrefValueMap* prefs = GetExtensionPrefValueMap(
          ext_id, extensions::kExtensionPrefsScopeRegularOnly);
      if (prefs->GetValue(key, &value)) {
        winner = i;
        winners_install_time = install_time;
        if (from_incognito)
          *from_incognito = false;
      }
      // Ignore the following prefs, because they're incognito-only.
      continue;
    }

    prefs = GetExtensionPrefValueMap(
        ext_id, extensions::kExtensionPrefsScopeIncognitoPersistent);
    if (prefs->GetValue(key, &value)) {
      winner = i;
      winners_install_time = install_time;
      if (from_incognito)
        *from_incognito = true;
    }

    prefs = GetExtensionPrefValueMap(
        ext_id, extensions::kExtensionPrefsScopeIncognitoSessionOnly);
    if (prefs->GetValue(key, &value)) {
      winner = i;
      winners_install_time = install_time;
      if (from_incognito)
        *from_incognito = true;
    }
  }
  return winner;
}

void ExtensionPrefValueMap::AddObserver(
    ExtensionPrefValueMap::Observer* observer) {
  observers_.AddObserver(observer);

  // Collect all currently used keys and notify the new observer.
  std::set<std::string> keys;
  ExtensionEntryMap::const_iterator i;
  for (i = entries_.begin(); i != entries_.end(); ++i)
    GetExtensionControlledKeys(*(i->second), &keys);

  std::set<std::string>::const_iterator j;
  for (j = keys.begin(); j != keys.end(); ++j)
    observer->OnPrefValueChanged(*j);
}

void ExtensionPrefValueMap::RemoveObserver(
    ExtensionPrefValueMap::Observer* observer) {
  observers_.RemoveObserver(observer);
}

void ExtensionPrefValueMap::NotifyInitializationCompleted() {
  FOR_EACH_OBSERVER(ExtensionPrefValueMap::Observer, observers_,
                    OnInitializationCompleted());
}

void ExtensionPrefValueMap::NotifyPrefValueChanged(
    const std::set<std::string>& keys) {
  std::set<std::string>::const_iterator i;
  for (i = keys.begin(); i != keys.end(); ++i)
    NotifyPrefValueChanged(*i);
}

void ExtensionPrefValueMap::NotifyPrefValueChanged(const std::string& key) {
  FOR_EACH_OBSERVER(ExtensionPrefValueMap::Observer, observers_,
                    OnPrefValueChanged(key));
}

void ExtensionPrefValueMap::NotifyOfDestruction() {
  FOR_EACH_OBSERVER(ExtensionPrefValueMap::Observer, observers_,
                    OnExtensionPrefValueMapDestruction());
}
