## 4.1 PRELIMINARIES

The Direct3D initialization process requires us to be familiar with some basic graphics
concepts and Direct3D types. We introduce these ideas and types in this section, so that
we do not have to digress when we cover the initialization process.

### 4.1.1 Direct3D 12 Overview

Direct3D is a low-level graphics API (application programming interface) used to control and program the GPU (graphics processing unit) from ur application, thereby allowing us to render virtual 3D worlds using hardware acceleration. For example, to submit a command to the GPU to clear a render target (e.g., the screen), we would call the Direct3D method ID3D12CommandList::ClearRenderTargetView.

The Direct3D layer and hardware drivers will translate the Direct3D commands into native machine instructions understood by the system’s GPU; thus, we do not have to worry about the specifics of the GPU, so long as it supports the Direct3D version we are using. To make this work, GPU vendors like NVIDIA, Intel, and AMD must work with the Direct3D team and provide compliant Direct3D drivers.

Direct3D 12 adds some new rendering features, but the main improvement over the previous version is that it has been redesigned to significantly reduce CPU overhead and improve multi-threading support. In order to achieve these performance goals, Direct3D 12 has become a much lower level API than Direct3D 11; it has less abstraction, requires additional manual "bookkeeping" from the developer, and more closely mirrors modern GPU architectures. The improved performance is, of course, the reward for using this more difficult API.

### 4.1.2 COM

### 4.1.3 Textures Formats

### 4.1.4 The Swap Chain and Page Flipping

### 4.1.5 Depth Buffering

### 4.1.6 Resources and Descriptors



## 4.2 CPU/GPU INTERACTION

### 4.2.1 The Command Queue and Command Lists

### 4.2.2 CPU/GPU Synchronization

Due to having two processors running in parallel, a number of synchronization issues appear.

