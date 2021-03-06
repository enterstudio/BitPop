// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_VIEWS_INDEXED_DB_INFO_VIEW_H_
#define CHROME_BROWSER_UI_VIEWS_INDEXED_DB_INFO_VIEW_H_

#include "chrome/browser/browsing_data/browsing_data_indexed_db_helper.h"
#include "ui/views/view.h"

namespace views {
class Textfield;
}

///////////////////////////////////////////////////////////////////////////////
// IndexedDBInfoView
//
//  Responsible for displaying a tabular grid of IndexedDB information.
class IndexedDBInfoView : public views::View {
 public:
  IndexedDBInfoView();
  virtual ~IndexedDBInfoView();

  // Update the display from the specified Local Storage info.
  void SetIndexedDBInfo(
      const BrowsingDataIndexedDBHelper::IndexedDBInfo&
      indexed_db_info);

  // Clears the cookie display to indicate that no or multiple local storages
  // are selected.
  void ClearIndexedDBDisplay();

  // Enables or disables the local storate property text fields.
  void EnableIndexedDBDisplay(bool enabled);

 protected:
  // views::View overrides:
  virtual void ViewHierarchyChanged(
      bool is_add, views::View* parent, views::View* child);

 private:
  // Set up the view layout
  void Init();

  // Individual property labels
  views::Textfield* origin_value_field_;
  views::Textfield* size_value_field_;
  views::Textfield* last_modified_value_field_;

  DISALLOW_COPY_AND_ASSIGN(IndexedDBInfoView);
};

#endif  // CHROME_BROWSER_UI_VIEWS_INDEXED_DB_INFO_VIEW_H_
