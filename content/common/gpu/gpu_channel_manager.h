// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_COMMON_GPU_GPU_CHANNEL_MANAGER_H_
#define CONTENT_COMMON_GPU_GPU_CHANNEL_MANAGER_H_

#include <vector>

#include "base/hash_tables.h"
#include "base/memory/ref_counted.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "base/message_loop_proxy.h"
#include "build/build_config.h"
#include "content/common/gpu/gpu_memory_manager.h"
#include "ipc/ipc_listener.h"
#include "ipc/ipc_sender.h"
#include "ui/gfx/native_widget_types.h"

namespace base {
class WaitableEvent;
}

namespace gfx {
class GLShareGroup;
}

namespace gpu {
namespace gles2 {
class MailboxManager;
class ProgramCache;
}
}

namespace IPC {
struct ChannelHandle;
}

class ChildThread;
class GpuChannel;
class GpuWatchdog;
struct GPUCreateCommandBufferConfig;
class SyncPointManager;

// A GpuChannelManager is a thread responsible for issuing rendering commands
// managing the lifetimes of GPU channels and forwarding IPC requests from the
// browser process to them based on the corresponding renderer ID.
//
// A GpuChannelManager can also be hosted in the browser process in single
// process or in-process GPU modes. In this case there is no corresponding
// GpuChildThread and this is the reason the GpuChildThread is referenced via
// a pointer to IPC::Sender, which can be implemented by other hosts to send
// IPC messages to the browser process IO thread on the GpuChannelManager's
// behalf.
class GpuChannelManager : public IPC::Listener,
                          public IPC::Sender,
                          public GpuMemoryManagerClient {
 public:
  GpuChannelManager(ChildThread* gpu_child_thread,
                    GpuWatchdog* watchdog,
                    base::MessageLoopProxy* io_message_loop,
                    base::WaitableEvent* shutdown_event);
  virtual ~GpuChannelManager();

  // Remove the channel for a particular renderer.
  void RemoveChannel(int client_id);

  // Listener overrides.
  virtual bool OnMessageReceived(const IPC::Message& msg) OVERRIDE;

  // Sender overrides.
  virtual bool Send(IPC::Message* msg) OVERRIDE;

  // GpuMemoryManagerClient overrides.
  virtual void AppendAllCommandBufferStubs(
      std::vector<GpuCommandBufferStubBase*>& stubs) OVERRIDE;

  void LoseAllContexts();

  base::WeakPtrFactory<GpuChannelManager> weak_factory_;

  int GenerateRouteID();
  void AddRoute(int32 routing_id, IPC::Listener* listener);
  void RemoveRoute(int32 routing_id);

  gpu::gles2::ProgramCache* program_cache();

  GpuMemoryManager* gpu_memory_manager() { return &gpu_memory_manager_; }

  GpuChannel* LookupChannel(int32 client_id);

  SyncPointManager* sync_point_manager() { return sync_point_manager_; }

 private:
  // Message handlers.
  void OnEstablishChannel(int client_id, bool share_context);
  void OnCloseChannel(const IPC::ChannelHandle& channel_handle);
  void OnVisibilityChanged(
      int32 render_view_id, int32 client_id, bool visible);
  void OnCreateViewCommandBuffer(
      const gfx::GLSurfaceHandle& window,
      int32 render_view_id,
      int32 client_id,
      const GPUCreateCommandBufferConfig& init_params);

  void OnLoseAllContexts();

  scoped_refptr<base::MessageLoopProxy> io_message_loop_;
  base::WaitableEvent* shutdown_event_;

  // Used to send and receive IPC messages from the browser process.
  ChildThread* gpu_child_thread_;

  // These objects manage channels to individual renderer processes there is
  // one channel for each renderer process that has connected to this GPU
  // process.
  typedef base::hash_map<int, scoped_refptr<GpuChannel> > GpuChannelMap;
  GpuChannelMap gpu_channels_;
  scoped_refptr<gfx::GLShareGroup> share_group_;
  scoped_refptr<gpu::gles2::MailboxManager> mailbox_manager_;
  GpuMemoryManager gpu_memory_manager_;
  GpuWatchdog* watchdog_;
  scoped_refptr<SyncPointManager> sync_point_manager_;
  scoped_ptr<gpu::gles2::ProgramCache> program_cache_;

  DISALLOW_COPY_AND_ASSIGN(GpuChannelManager);
};

#endif  // CONTENT_COMMON_GPU_GPU_CHANNEL_MANAGER_H_
