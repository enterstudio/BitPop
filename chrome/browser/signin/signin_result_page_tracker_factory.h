// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_SIGNIN_SIGNIN_RESULT_PAGE_TRACKER_FACTORY_H_
#define CHROME_BROWSER_SIGNIN_SIGNIN_RESULT_PAGE_TRACKER_FACTORY_H_

#include "base/memory/singleton.h"
#include "chrome/browser/profiles/profile_keyed_service_factory.h"

class SigninResultPageTracker;
class Profile;

// Singleton that owns all SigninManagers and associates them with
// Profiles. Listens for the Profile's destruction notification and cleans up
// the associated SigninManager.
class SigninResultPageTrackerFactory : public ProfileKeyedServiceFactory {
 public:
  // Returns the instance of SigninManager associated with this profile
  // (creating one if none exists). Returns NULL if this profile cannot have a
  // SigninManager (for example, if |profile| is incognito).
  static SigninResultPageTracker* GetForProfile(Profile* profile);

  // Returns an instance of the SigninManagerFactory singleton.
  static SigninResultPageTrackerFactory* GetInstance();

 private:
  friend struct DefaultSingletonTraits<SigninResultPageTrackerFactory>;

  SigninResultPageTrackerFactory();
  virtual ~SigninResultPageTrackerFactory();

  // ProfileKeyedServiceFactory:
  virtual ProfileKeyedService* BuildServiceInstanceFor(
      Profile* profile) const OVERRIDE;
};

#endif  // CHROME_BROWSER_SIGNIN_SIGNIN_RESULT_PAGE_TRACKER_FACTORY_H_
