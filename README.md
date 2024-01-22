# KYBER-on-PYNQ
CRYSTALS-Kyber is a Lattice-based Post Quantum Cryptography protocol chosen for standardization by NIST.  </br>

This repository contains Hardware-Software co-design of CRYSTALS-Kyber based Public Key Cryptography system on PYNQ-Z2 FPGA. </br>
Number Theoretic Transform (NTT) based Polynomial Multiplication unit is designed using Vivado HLS for hardware acceleration. The accelerator is interfaced with Zynq Processing System, which runs the software part and controls data transfer to/from the accelerator.
