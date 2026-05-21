---
name: Hardware / RTL Issue
about: Report an issue with RTL, FPGA behavior, SoC design, timing, simulation, or hardware integration
title: "[RTL/HW] "
labels: hardware, rtl, needs-triage
assignees: ''
---

## Issue Summary

Describe the hardware, RTL, FPGA, or simulation issue.

## Affected Area

Select all that apply:

- [ ] Verilog / VHDL module
- [ ] CPU / SoC design
- [ ] Memory / bus interface
- [ ] MMIO / peripheral interface
- [ ] VGA / display output
- [ ] UART / SPI / I2C / GPIO
- [ ] Button/switch input
- [ ] Seven-segment display
- [ ] Clock/reset logic
- [ ] Constraints file
- [ ] FPGA synthesis/implementation
- [ ] Testbench/simulation
- [ ] Timing issue
- [ ] Other:

## Target Hardware

- Board:
- FPGA:
- Clock frequency:
- Toolchain/version:
- External devices/peripherals:

## Relevant Module(s)

List affected files/modules.

```text
Example:
rtl/cpu_core.v
rtl/mmio.v
constraints/NexysA7.xdc
```

## Expected Behavior

Describe the expected hardware or simulation behavior.

## Actual Behavior

Describe what actually happens.

## Reproduction Steps

1.
2.
3.

## Simulation or Hardware Evidence

Add logs, waveform notes, timing reports, screenshots, or board observations.

```text
Paste simulator/synthesis/timing output here
```

## Possible Cause

Optional: describe what you think may be causing the issue.

## Additional Context

Add anything else that may help debug the issue.