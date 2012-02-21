// Copyright (c) 2012 House of Life Property Ltd. All rights reserved.
// Copyright (c) 2012 Crystalnix <vgachkaylo@crystalnix.com>
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_FACEBOOK_CHAT_CHAT_ITEM_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_FACEBOOK_CHAT_CHAT_ITEM_VIEW_H_

#pragma once

#include <list>

#include "base/timer.h"
#include "chrome/browser/facebook_chat/facebook_chat_item.h"
#include "chrome/browser/ui/views/bubble/bubble.h"
#include "chrome/browser/ui/views/facebook_chat/chat_popup.h"
#include "ui/base/animation/animation_delegate.h"
#include "views/controls/button/button.h"
#include "views/view.h"

class ChatbarView;
class ChatNotificationPopup;

namespace gfx {
class Bitmap;
class Image;
}

namespace ui {
class SlideAnimation;
}

namespace views {
class TextButton;
class ImageButton;
}

class ChatItemView : public views::ButtonListener,
                     public views::View,
                     public FacebookChatItem::Observer,
                     public ui::AnimationDelegate,
                     public ChatPopup::Observer,
                     public BubbleDelegate {
public:
  ChatItemView(FacebookChatItem *model, ChatbarView *chatbar);
  virtual ~ChatItemView();

  // views::ButtonListener protocol
  virtual void ButtonPressed(views::Button* sender, const views::Event& event) OVERRIDE;

  // Overridden from views::View:
  virtual void Layout() OVERRIDE;
  virtual gfx::Size GetPreferredSize() OVERRIDE;
  virtual void OnMouseEntered(const views::MouseEvent& event) OVERRIDE;
  virtual void OnMouseExited(const views::MouseEvent& event) OVERRIDE;

  // FacebookChatItem::Observer protocol
  virtual void OnChatUpdated(FacebookChatItem *source) OVERRIDE;

  // ui::AnimationDelegate implementation.
  virtual void AnimationProgressed(const ui::Animation* animation) OVERRIDE;

  // ChatPopup::Observer implementation
  virtual void ChatPopupIsClosing(ChatPopup* popup) OVERRIDE;

  // BubbleDelegate implementation
  virtual void BubbleClosing(Bubble* bubble, bool closed_by_escape) OVERRIDE;
  virtual bool CloseOnEscape() OVERRIDE { return false; }
  virtual bool FadeInOnShow() OVERRIDE { return true; }
  
  void Close(bool should_animate);

  void ActivateChat();
  void NotifyUnread();

  gfx::Rect RectForChatPopup();
  gfx::Rect RectForNotificationPopup();

  const FacebookChatItem* GetModel() const;
protected:
  // Overridden from views::View:
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE;

  virtual void UpdateNotificationIcon();

private:

  void StatusChanged();
  void TimerFired();

  FacebookChatItem *model_;
  
  ChatbarView *chatbar_;
  
  views::TextButton *openChatButton_;

  views::ImageButton *close_button_;

  SkColor close_button_bg_color_;

  ChatPopup *chat_popup_;
  ChatNotificationPopup* notification_popup_;

  typedef base::OneShotTimer<ChatItemView> ChatTimer;
  typedef std::list<ChatTimer*> TimerList;
  TimerList timers_;

  bool isMouseOverNotification_;

  SkBitmap *notification_icon_;
};

#endif  // CHROME_BROWSER_UI_VIEWS_FACEBOOK_CHAT_CHAT_ITEM_VIEW_H_