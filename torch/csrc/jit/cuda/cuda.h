#include <aten/src/ATen/cuda/CUDAEvent.h>
#include <c10/core/Device.h>
#include <c10/cuda/CUDAStream.h>
#include <torch/custom_class.h>

namespace torch {
namespace jit {

class CUDAEvent;
class CUDAStream final : public CustomClassHolder {
 public:
  CUDAStream(int64_t device = -1, int64_t priority = 0) {
    constexpr int64_t PRIORITY_INDEX = 50;
    stream_ = std::make_unique<c10::cuda::CUDAStream>(
        c10::cuda::getStreamFromPool(priority > PRIORITY_INDEX, device));
  }

  bool query() {
    return stream_->query();
  }

  c10::intrusive_ptr<CUDAEvent> recordEvent(
      c10::intrusive_ptr<CUDAEvent> event);

  void synchronize() {
    stream_->synchronize();
  }

  void waitEvent(c10::intrusive_ptr<CUDAEvent> event);

  void waitStream(c10::intrusive_ptr<CUDAStream> stream);

  /// Get the CUDA device index that this stream is associated with.
  int64_t device_index() const {
    return stream_->device_index();
  }

  /// Get the full Device that this stream is associated with.  The Device
  /// is guaranteed to be a CUDA device.
  c10::Device device() const {
    return stream_->device();
  }

  /// Return the stream ID corresponding to this particular stream.
  int64_t id() const {
    return stream_->id();
  }

  int64_t pack() const noexcept {
    return stream_->pack();
  }

  /// Set the current or the default stream for the given device
  /// depending on the bool value is_current. This is a wrapper function
  /// which sets the stream for torch.jit.CUDStreams.
  void setStream(int64_t idx = -1, bool is_current = false) {
    if (is_current) {
      auto s = c10::cuda::getCurrentCUDAStream(idx);
      stream_ = std::make_unique<c10::cuda::CUDAStream>(s);
    } else {
      auto s = c10::cuda::getDefaultCUDAStream(idx);
      stream_ = std::make_unique<c10::cuda::CUDAStream>(s);
    }
  }

 private:
  std::unique_ptr<c10::cuda::CUDAStream> stream_;
  friend class CUDAEvent;
};

class CUDAEvent final : public CustomClassHolder {
 public:
  CUDAEvent(
      bool enable_timing = false,
      bool blocking = false,
      bool interprocess = false) {
    int flags = cudaEventDisableTiming;
    if (enable_timing) {
      flags = cudaEventDefault;
    }
    if (blocking) {
      flags |= cudaEventBlockingSync;
    }
    if (interprocess) {
      TORCH_CHECK(!enable_timing);
      flags |= cudaEventInterprocess;
    }

    event_ = std::make_unique<at::cuda::CUDAEvent>(flags);
  }

  double elapsedTime(c10::intrusive_ptr<CUDAEvent> end) {
    return event_->elapsed_time(*end->event_);
  }

  std::string ipcHandle() {
    cudaIpcEventHandle_t handle;
    event_->ipc_handle(&handle);
    std::string str_handle((const char*)&handle, sizeof(handle));
    return str_handle;
  }

  bool query() {
    return event_->query();
  }

  void record(c10::intrusive_ptr<CUDAStream> stream);

  void synchronize() {
    event_->synchronize();
  }
  void wait(c10::intrusive_ptr<CUDAStream> stream);

 private:
  void recordInternal(CUDAStream* stream);
  std::unique_ptr<at::cuda::CUDAEvent> event_;

  friend class CUDAStream;
};

c10::intrusive_ptr<CUDAEvent> CUDAStream::recordEvent(
    c10::intrusive_ptr<CUDAEvent> event) {
  if (!event) {
    event = c10::make_intrusive<CUDAEvent>();
  }

  event->recordInternal(this);
  return event;
}

void CUDAStream::waitEvent(c10::intrusive_ptr<CUDAEvent> event) {
  event->event_->block(*stream_);
}

void CUDAStream::waitStream(c10::intrusive_ptr<CUDAStream> stream) {
  auto ev = c10::make_intrusive<CUDAEvent>();
  stream->recordEvent(ev);
  waitEvent(ev);
}

void CUDAEvent::record(c10::intrusive_ptr<CUDAStream> stream) {
  event_->record(*stream->stream_);
}

void CUDAEvent::recordInternal(CUDAStream* stream) {
  event_->record(*stream->stream_);
}

void CUDAEvent::wait(c10::intrusive_ptr<CUDAStream> stream) {
  event_->block(*stream->stream_);
}

TORCH_LIBRARY(cuda, m) {
  auto stream_class = m.class_<torch::jit::CUDAStream>("Stream").def(
      torch::init<int64_t, int64_t>());
  auto event_class = m.class_<torch::jit::CUDAEvent>("Event").def(
      torch::init<bool, bool, bool>());

  stream_class.def("query", &CUDAStream::query)
      .def("record_event", &CUDAStream::recordEvent)
      .def("synchronize", &CUDAStream::synchronize)
      .def("wait_event", &CUDAStream::waitEvent)
      .def("wait_stream", &CUDAStream::waitStream)
      .def("setstream", &CUDAStream::setStream)
      .def("device_index", &CUDAStream::device_index)
      .def("device", &CUDAStream::device)
      .def("pack", &CUDAStream::pack)
      .def("id", &CUDAStream::id);

  event_class.def("elapsed_time", &CUDAEvent::elapsedTime)
      .def("ipc_handle", &CUDAEvent::ipcHandle)
      .def("query", &CUDAEvent::query)
      .def("record", &CUDAEvent::record)
      .def("synchronize", &CUDAEvent::synchronize)
      .def("wait", &CUDAEvent::wait);
};

} // namespace jit
} // namespace torch