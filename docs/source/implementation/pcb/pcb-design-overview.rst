PCB Design Overview
===================

This page provides a high-level overview of the PCB design process for the Ma Bell Gateway project.

Design Philosophy
-----------------

The goal is to produce a robust, easy-to-assemble printed circuit board that faithfully implements the Ma Bell Gateway circuit while maintaining signal integrity, power isolation, and modularity.  
Particular attention is given to proper grounding, clear separation of analog and digital domains, and ease of hand-assembly for prototyping and hobbyist builds.

Design Tools
------------

PCB design for this project is performed using **KiCad**, an open-source electronic design automation (EDA) suite.  
KiCad provides schematic capture, PCB layout, 3D visualization, and output generation for fabrication and assembly.  
All project files (schematics, board layouts, and libraries) are maintained in the project repository.

Workflow Overview
-----------------

1. **Schematic Capture:**  
   The complete circuit is drawn in KiCad’s schematic editor, with careful attention to symbol and footprint assignment.

2. **PCB Layout:**  
   Components are placed and routed in KiCad PCBNew, with attention to:
   - Star grounding and ground planes
   - Segregation of analog, digital, and high-voltage sections
   - Proper trace widths for power and signal paths
   - Placement of protection, filtering, and connectors

3. **Design Rule Checks (DRC):**  
   Layout is verified using KiCad’s built-in DRC tools to catch clearance, net, and layout errors.

4. **3D Visualization:**  
   The finished board is reviewed in 3D to check mechanical fit and connector alignment.

5. **Manufacturing Outputs:**  
   Gerber files, drill files, BOM, and pick-and-place files are exported for PCB fabrication and assembly.

Project Repository
------------------

All KiCad source files, as well as outputs for manufacturing and assembly, are tracked in the project’s Git repository:

- Schematic files (`.kicad_sch`)
- PCB layout files (`.kicad_pcb`)
- Symbol and footprint libraries
- BOM, Gerbers, assembly drawings

See the repository documentation for file organization and version control practices.

Next Steps
----------

Refer to the subpages in this section for detailed information on the PCB workflow, layout strategies, assembly guidance, and troubleshooting tips.

SLIC Circuit Layout Guidelines
------------------------------

Proper PCB layout is critical for the SLIC circuit to function reliably and meet noise/EMI requirements. Follow these guidelines during board design.

General Layout Strategy
^^^^^^^^^^^^^^^^^^^^^^^

1. **Functional Zones:**

   - Protection zone (fuses, bridge, MOV) near phone jack
   - SLIC zone (IC and immediate passives) in center
   - Audio zone (codec and coupling) near SLIC
   - Power supply zone (buck converters, decoupling) separate from audio

2. **Signal Flow:**

   ::

       Phone Jack ──► Protection ──► SLIC ──► Audio Codec ──► ESP32
                                      │
                                   Power Supply

   Arrange components to follow this left-to-right signal flow on PCB.

Grounding
^^^^^^^^^

**Star Ground Topology:**

The three SLIC ground pins (AG, DG, BG) must connect at a single star ground point as close to the SLIC as possible.

::

    ┌─────────────────────────────────┐
    │        SLIC IC (Top View)        │
    │                                  │
    │   Pin 23 (AG) ──┐               │
    │   Pin 6  (DG) ──┼───► ★ Star    │
    │   Pin 12 (BG) ──┘      GND Pad  │
    │                        (Copper)  │
    └─────────────────────────────────┘
             │
             └──► System Ground Plane

- Use wide traces (>50 mil) or copper pour for star ground connection
- Avoid running return currents from other circuits through SLIC ground
- Connect star ground to main PCB ground plane at single point

Power Supply Decoupling
^^^^^^^^^^^^^^^^^^^^^^^^

- **Ceramic capacitors (100nF):** Place within 0.5" (12mm) of VB+ pin
- **Bulk capacitors (10µF):** Place within 1" (25mm) of VB+ and VB- pins
- **Via placement:** Use multiple vias (2-4) for power/ground connections to reduce inductance

High Voltage Clearances
^^^^^^^^^^^^^^^^^^^^^^^^

For traces carrying -48V, +12V, or ring voltage (90V AC):

- **Trace spacing:** Minimum 2mm (80 mil) between high voltage and low voltage traces
- **Creepage:** Maintain 2.5mm creepage distance for 150V rating
- **Clearance:** Air gap ≥2mm for 150V
- **Solder mask:** Use solder mask as additional barrier but don't rely on it alone for isolation

Audio Path Layout
^^^^^^^^^^^^^^^^^

Audio traces (SLIC TX/RX to codec) should be:

- **Short and direct:** Minimize trace length (<3 inches ideal)
- **Differential routing:** Route TX+ and TX- as differential pair (if applicable)
- **Guard traces:** Surround audio traces with ground guard traces
- **Ground plane:** Provide solid ground plane under audio signals
- **Separation:** Keep audio traces away from digital signals (I2S clock, data) and switching power supplies

Digital Signal Integrity
^^^^^^^^^^^^^^^^^^^^^^^^^

For ESP32 GPIO and I2S signals:

- **Trace impedance:** Target 50Ω for high-speed signals (I2S BCLK, LRCLK)
- **Length matching:** Match I2S data and clock trace lengths within 0.5"
- **Termination:** Series 22-33Ω resistors on I2S signals near ESP32 if trace length >3"
- **Crosstalk:** Separate parallel traces by 3x trace width minimum

Protection Circuit Placement
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Place protection components (fuses, bridge, MOV) as close to the phone jack as possible:

::

    Phone Jack ──[ <0.5" ]──► Fuses ──[ <1" ]──► Bridge/MOV ──► SLIC

- Minimizes unprotected trace length
- Reduces chances of surge coupling to other circuits
- Use wide traces (>20 mil / 0.5mm) for TIP/RING to handle fault currents

Layer Stackup
^^^^^^^^^^^^^

**Recommended 4-layer stackup:**

::

    Layer 1: Signal + Components (top)
    Layer 2: Ground Plane
    Layer 3: Power Plane (+12V, +3.3V, -48V zones)
    Layer 4: Signal + Components (bottom)

**For 2-layer boards:**

- Top: Signals and components
- Bottom: Ground pour with power routing

Thermal Considerations
^^^^^^^^^^^^^^^^^^^^^^

- SLIC IC can dissipate 1-2W during ringing
- Provide thermal relief with copper pour connected to ground pins
- If using DIP package, consider adding heat sink or thermal pad

Component Placement Summary
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

::

    ┌──────────────────────────────────────────────────┐
    │  Phone Jack (J1)                                  │
    │       │                                           │
    │   [Protection Zone]                               │
    │    F1, F2, BR1, MOV                               │
    │       │                                           │
    │    ┌──────────────────┐                          │
    │    │   SLIC IC (U1)   │   [Audio Zone]          │
    │    │   HC-5504B-5     │    Codec, coupling       │
    │    │                  ├──────────┐               │
    │    │  Passives around │          │               │
    │    │  (C2, C3, C4,    │      Codec IC            │
    │    │   R_TIP, R_RING) │          │               │
    │    └──────────────────┘          │               │
    │       │                           │               │
    │    [Power Zone]              I2S signals          │
    │    Buck converters           to ESP32 ───────────►│
    │    Decoupling caps                                │
    │                                                   │
    └───────────────────────────────────────────────────┘

Testing & Validation
^^^^^^^^^^^^^^^^^^^^

After PCB assembly:

1. Visual inspection for solder bridges, polarity
2. Continuity check: Star ground, power rails
3. Power-on test: Measure +12V, -48V at SLIC pins
4. Off-hook test: Connect phone, lift handset, verify SHD signal
5. Audio loopback: Test TX/RX paths with signal generator/scope
6. Protection test: Apply controlled overvoltage to verify MOV clamping

