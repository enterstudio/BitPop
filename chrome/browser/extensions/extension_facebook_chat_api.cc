// Copyright (c) 2011 House of Life Property ltd. All rights reserved.
// Copyright (c) 2011 Crystalnix <vgachkaylo@crystalnix.com>
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/extension_facebook_chat_api.h"

#include "base/values.h"
#include "chrome/common/extensions/extension_error_utils.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_window.h"
#include "content/browser/tab_contents/tab_contents.h"
#include "content/common/notification_service.h"
#include "chrome/common/pref_names.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/facebook_chat/facebook_chat_create_info.h"

namespace {
// Errors.
const char kInvalidArguments[] =
  "Invalid arguments passed to function.";
const char kNoCurrentWindowError[] = "No current browser window was found";
} // namespace

// List is considered empty if it is actually empty or contains just one value,
// either 'null' or 'undefined'.
static bool IsArgumentListEmpty(const ListValue* arguments) {
  if (arguments->empty())
    return true;
  if (arguments->GetSize() == 1) {
    Value* first_value = 0;
    if (!arguments->Get(0, &first_value))
      return true;
    if (first_value->GetType() == Value::TYPE_NULL)
      return true;
  }
  return false;
}

bool SetFriendsSidebarVisibleFunction::RunImpl() {
  if (!args_.get())
    return false;

  bool is_visible = true;
  if (IsArgumentListEmpty(args_.get())) {
    error_ = kInvalidArguments;
    return false;
  } else {
    EXTENSION_FUNCTION_VALIDATE(args_->GetBoolean(0, &is_visible));
  }

  Browser* browser = GetCurrentBrowser();
  if (!browser || !browser->window()) {
    error_ = kNoCurrentWindowError;
    return false;
  }

  PrefService *prefService = browser->profile()->GetPrefs();
  prefService->SetBoolean(prefs::kFacebookShowFriendsList, is_visible);

  return true;
}

bool GetFriendsSidebarVisibleFunction::RunImpl() {
  if (!args_.get())
    return false;

  Browser* browser = GetCurrentBrowser();
  if (!browser || !browser->window()) {
    error_ = kNoCurrentWindowError;
    return false;
  }

  result_.reset(Value::CreateBooleanValue(
        browser->window()->IsFriendsSidebarVisible()));
  return true;
}

bool AddChatFunction::RunImpl() {
  if (!args_.get())
    return false;

  std::string jid(""), username(""), status("");

  if (IsArgumentListEmpty(args_.get()) || args_->GetSize() != 3) {
    error_ = kInvalidArguments;
    return false;
  } else {
    EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &jid));
    EXTENSION_FUNCTION_VALIDATE(args_->GetString(1, &username));
    EXTENSION_FUNCTION_VALIDATE(args_->GetString(2, &status));
  }

  // FacebookChatItem::Status statusValue = FacebookChatItem::AVAILABLE;
  // if (status != "available")
  //   statusValue = FacebookChatItem::OFFLINE;

  Browser* browser = GetCurrentBrowser();
  if (!browser) {
    error_ = kNoCurrentWindowError;
    return false;
  }

  NotificationService::current()->Notify(
      NotificationType::FACEBOOK_CHATBAR_ADD_CHAT,
      Source<Profile>(browser->profile()),
      Details<FacebookChatCreateInfo>(
        new FacebookChatCreateInfo(jid, username, status)));

  return true;
}


