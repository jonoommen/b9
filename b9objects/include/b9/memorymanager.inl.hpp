#if !defined(B9_MEMORYMANAGER_INL_HPP_)
#define B9_MEMORYMANAGER_INL_HPP_

#include <b9/allocator.hpp>
#include <b9/runtime.hpp>
#include <b9/traverse.hpp>

#include <b9/context.inl.hpp>

namespace b9 {

inline MemoryManager::MemoryManager(ProcessRuntime& runtime)
    : runtime_(runtime) {

  Thread self(runtime.platform().thread());
  initOmrVm();
  initOmrGc();

  StartupContext cx(*this);
  initGlobals(cx);
}

inline MemoryManager::~MemoryManager() {
  Thread self(runtime().platform().thread());
  // TODO: Shut down the heap (requires a thread (boo!!))
  omr_detach_vm_from_runtime(&omrVm());
}

inline void MemoryManager::initOmrVm() {
  memset(&omrVm_, 0, sizeof(OMR_VM));
  omrVm_._runtime = &runtime_.omrRuntime();
  omrVm_._language_vm = this;

  auto e = omr_attach_vm_to_runtime(&omrVm_);
  if (e != 0) {
    throw PlatformError(e);
  }
}

inline void MemoryManager::initOmrGc() {
  MM_StartupManagerImpl startupManager(&omrVm_);
  auto e = OMR_GC_IntializeHeapAndCollector(&omrVm_, &startupManager);
  if (e != 0) {
    throw PlatformError(e);
  }
}

inline void MemoryManager::initGlobals(StartupContext& cx) {
  globals_.mapMap = allocateMapMap(cx);
  globals_.emptyObjectMap = allocateEmptyObjectMap(cx);
}

inline void MemoryManager::visitRoots(Context& cx, Visitor& visitor) {
  visitor.rootEdge(cx, this, globals_.mapMap);
  visitor.rootEdge(cx, this, globals_.emptyObjectMap);
};

}  // namespace b9

#endif  // B9_MEMORYMANAGER_INL_HPP_
