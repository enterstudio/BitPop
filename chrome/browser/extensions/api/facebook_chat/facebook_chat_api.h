// Copyright (c) 2011 House of Life Property ltd. All rights reserved.
// Copyright (c) 2011 Crystalnix <vgachkaylo@crystalnix.com>
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_FACEBOOK_CHAT_API_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_FACEBOOK_CHAT_API_H_

#include <string>
#include "chrome/browser/extensions/extension_function.h"

class TabContents;

// Base class for facebook chat function APIs.
// class FacebookChatFunction : public SyncExtensionFunction {
//  public:
//   virtual bool RunImpl();
//  private:
//   virtual bool RunImpl(TabContents* tab,
//                        const std::string& content_id,
//                        const DictionaryValue& details) = 0;
// };

class SetFriendsSidebarVisibleFunction : public SyncExtensionFunction {
 public:
  virtual bool RunImpl() OVERRIDE;
  DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.setFriendsSidebarVisible");
 protected:
  virtual ~SetFriendsSidebarVisibleFunction() {}
};

class GetFriendsSidebarVisibleFunction : public SyncExtensionFunction {
 public:
  virtual bool RunImpl() OVERRIDE;
  DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.getFriendsSidebarVisible");
 protected:
  virtual ~GetFriendsSidebarVisibleFunction() {}
};

class AddChatFunction : public SyncExtensionFunction {
  public:
    virtual bool RunImpl() OVERRIDE;
    DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.addChat");
  protected:
    virtual ~AddChatFunction() {}
};

class NewIncomingMessageFunction: public SyncExtensionFunction {
  public:
    virtual bool RunImpl() OVERRIDE;
    DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.newIncomingMessage");
  protected:
    virtual ~NewIncomingMessageFunction() {}
};

class LoggedOutFacebookSessionFunction: public SyncExtensionFunction {
  public:
    virtual bool RunImpl() OVERRIDE;
    DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.loggedOutFacebookSession");
  protected:
    virtual ~LoggedOutFacebookSessionFunction() {}
};

class LoggedInFacebookSessionFunction: public SyncExtensionFunction {
  public:
    virtual bool RunImpl() OVERRIDE;
    DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.loggedInFacebookSession");
  protected:
    virtual ~LoggedInFacebookSessionFunction() {}

};

class SetGlobalMyUidForProfileFunction : public SyncExtensionFunction {
  public:
    virtual bool RunImpl() OVERRIDE;
    DECLARE_EXTENSION_FUNCTION_NAME("bitpop.facebookChat.setGlobalMyUidForProfile");
  protected:
    virtual ~SetGlobalMyUidForProfileFunction() {}
};

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_FACEBOOK_CHAT_API_H_

