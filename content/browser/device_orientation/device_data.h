// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_BROWSER_DEVICE_ORIENTATION_DEVICE_DATA_H_
#define CONTENT_BROWSER_DEVICE_ORIENTATION_DEVICE_DATA_H_

#include "base/memory/ref_counted.h"
#include "content/common/content_export.h"

namespace IPC {
class Message;
}

namespace device_orientation {

class CONTENT_EXPORT DeviceData :
    public base::RefCountedThreadSafe<DeviceData> {
 public:
  enum Type {
    kTypeOrientation,
    kTypeTest
  };

  virtual IPC::Message* CreateIPCMessage(int render_view_id) const = 0;
  virtual bool ShouldFireEvent(const DeviceData* other) const = 0;

 protected:
  DeviceData() {}
  virtual ~DeviceData() {}

 private:
  friend class base::RefCountedThreadSafe<DeviceData>;

  DISALLOW_COPY_AND_ASSIGN(DeviceData);
};

}  // namespace device_orientation

#endif  // CONTENT_BROWSER_DEVICE_ORIENTATION_DEVICE_DATA_H_
