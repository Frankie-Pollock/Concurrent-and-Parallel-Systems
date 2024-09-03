# Concurrent and Parallel Systems (SET10108)

## Semester 1 – 2023/2024

### Course Overview

This repository contains the coursework for the **Concurrent and Parallel Systems** module. The module focuses on developing skills in designing, implementing, and evaluating concurrent and parallel systems using C++ and other relevant tools. The coursework is divided into two main projects:

1. [Image Fever](#project-1-image-fever)
2. [N-Queens Problem](#project-2-n-queens-problem)

Both projects emphasize parallelization techniques and require a detailed report accompanying the implementation.

---

## Project 1: Image Fever

### Overview

**Objective:** Develop a C++ application that sorts images by color temperature based on their median hue. The sorting process must be progressive to maintain a smooth user experience.

**Key Requirements:**
- Convert images from RGB to HSV color space and compute the median hue.
- Sort images progressively, ensuring the UI remains responsive.
- Implement parallelism to handle image loading, processing, and sorting in background threads.

### Technical Details
- **Programming Language:** C++
- **Libraries:** Use `stb_image` for image loading.
- **Performance Measurement:** Compare the startup and sorting times with and without parallelization.

### Report Structure
- **Analysis:** Identify parallelizable tasks and shared resources.
- **Methodology:** Justify the chosen parallelization techniques.
- **Results:** Evaluate performance, include hardware setup, and present findings with graphs.

---

## Project 2: N-Queens Problem

### Overview

**Objective:** Modify an existing N-Queens problem implementation to run in parallel using OpenMP and GPU (CUDA/OpenCL).

**Key Requirements:**
- Reformulate the recursive N-Queens algorithm to work without recursion.
- Implement and compare three variations: Serial, OpenMP, and GPU.
- Measure and compare the performance of each approach for different values of N.

### Technical Details
- **Programming Language:** C++
- **Parallelization Tools:** OpenMP for CPU parallelism, CUDA/OpenCL for GPU parallelism.
- **Performance Measurement:** Compare execution times across serial, OpenMP, and GPU implementations.

### Report Structure
- **OpenMP Implementation:** Describe the parallelization approach, results, and any variants tested.
- **GPU Implementation:** Detail the GPU-based parallelization, results, and any tested configurations.
- **Overall Findings:** Compare and discuss the three approaches, including hardware setup and performance graphs.
