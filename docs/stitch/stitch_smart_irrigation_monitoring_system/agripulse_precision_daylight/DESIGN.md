---
name: AgriPulse Precision Daylight
colors:
  surface: '#faf8ff'
  surface-dim: '#d2d9f4'
  surface-bright: '#faf8ff'
  surface-container-lowest: '#ffffff'
  surface-container-low: '#f2f3ff'
  surface-container: '#eaedff'
  surface-container-high: '#e2e7ff'
  surface-container-highest: '#dae2fd'
  on-surface: '#131b2e'
  on-surface-variant: '#3d4a42'
  inverse-surface: '#283044'
  inverse-on-surface: '#eef0ff'
  outline: '#6d7a72'
  outline-variant: '#bccac0'
  surface-tint: '#006c4a'
  primary: '#006948'
  on-primary: '#ffffff'
  primary-container: '#00855d'
  on-primary-container: '#f5fff7'
  inverse-primary: '#68dba9'
  secondary: '#006c49'
  on-secondary: '#ffffff'
  secondary-container: '#6cf8bb'
  on-secondary-container: '#00714d'
  tertiary: '#006194'
  on-tertiary: '#ffffff'
  tertiary-container: '#007bb9'
  on-tertiary-container: '#fdfcff'
  error: '#ba1a1a'
  on-error: '#ffffff'
  error-container: '#ffdad6'
  on-error-container: '#93000a'
  primary-fixed: '#85f8c4'
  primary-fixed-dim: '#68dba9'
  on-primary-fixed: '#002114'
  on-primary-fixed-variant: '#005137'
  secondary-fixed: '#6ffbbe'
  secondary-fixed-dim: '#4edea3'
  on-secondary-fixed: '#002113'
  on-secondary-fixed-variant: '#005236'
  tertiary-fixed: '#cce5ff'
  tertiary-fixed-dim: '#93ccff'
  on-tertiary-fixed: '#001d31'
  on-tertiary-fixed-variant: '#004b73'
  background: '#faf8ff'
  on-background: '#131b2e'
  surface-variant: '#dae2fd'
typography:
  headline-xl:
    fontFamily: Space Grotesk
    fontSize: 32px
    fontWeight: '600'
    lineHeight: 40px
    letterSpacing: -0.02em
  headline-xl-mobile:
    fontFamily: Space Grotesk
    fontSize: 24px
    fontWeight: '600'
    lineHeight: 32px
    letterSpacing: -0.01em
  headline-lg:
    fontFamily: Space Grotesk
    fontSize: 24px
    fontWeight: '600'
    lineHeight: 32px
    letterSpacing: -0.01em
  headline-md:
    fontFamily: Space Grotesk
    fontSize: 18px
    fontWeight: '600'
    lineHeight: 24px
  data-display:
    fontFamily: Space Grotesk
    fontSize: 28px
    fontWeight: '700'
    lineHeight: 32px
    letterSpacing: -0.02em
  data-metric:
    fontFamily: Space Grotesk
    fontSize: 20px
    fontWeight: '600'
    lineHeight: 26px
  body-lg:
    fontFamily: Hanken Grotesk
    fontSize: 16px
    fontWeight: '400'
    lineHeight: 24px
  body-md:
    fontFamily: Hanken Grotesk
    fontSize: 14px
    fontWeight: '400'
    lineHeight: 20px
  body-sm:
    fontFamily: Hanken Grotesk
    fontSize: 12px
    fontWeight: '400'
    lineHeight: 16px
  label-md:
    fontFamily: Space Grotesk
    fontSize: 13px
    fontWeight: '500'
    lineHeight: 16px
    letterSpacing: 0.01em
  label-sm:
    fontFamily: Space Grotesk
    fontSize: 11px
    fontWeight: '600'
    lineHeight: 14px
    letterSpacing: 0.04em
  code-telemetry:
    fontFamily: Space Grotesk
    fontSize: 12px
    fontWeight: '500'
    lineHeight: 16px
rounded:
  sm: 0.125rem
  DEFAULT: 0.25rem
  md: 0.375rem
  lg: 0.5rem
  xl: 0.75rem
  full: 9999px
spacing:
  gutter: 1rem
  gutter-lg: 1.5rem
  margin: 1rem
  margin-md: 1.5rem
  margin-lg: 2rem
  space-xs: 0.25rem
  space-sm: 0.5rem
  space-md: 0.75rem
  space-lg: 1rem
  space-xl: 1.5rem
---

## Brand & Style

This design system delivers an operational daylight environment tailored for real-time agronomic telemetry, sensor fleets, and automated irrigation systems. The aesthetic reflects high-precision environmental intelligence: crisp, utilitarian, clean, and uncompromisingly legible under high ambient light conditions such as direct field sunlight or bright operations control rooms.

The design movement synthesizes **Modern Technical Minimalism** with **Precision Data Density**. It departs from dark-mode hacker tropes to embrace clinical clarity: pure optic whites, structured slate containers, hairline architectural dividers, and vital emerald accents representing biological health, active flow rates, and sensor operationality.

### Emotional Response
- **Trust & Calibration:** Interface components feel engineered, accurate, and instrument-grade.
- **Immediate Vitality:** Energetic, natural emerald highlights signal live automated telemetry without visual fatigue.
- **Cognitive Relief:** Structured spacing and high-contrast typographic scales prevent sensory overload across large-scale IoT fleet views.

## Colors

The color palette centers on a daylight hierarchy, combining optic whites and cool slates to support rapid scanning of multi-node telemetry.

### Palette Architecture
- **Primary Emerald (`#059669`):** Primary interactive triggers, active valve states, normal soil moisture indicators, and healthy field zones.
- **Secondary Mint Emerald (`#10b981`):** Telemetry sparklines, active hardware toggles, live status pulses, and positive trending metrics.
- **Tertiary Cyan/Blue (`#0284c7`):** Hydro-telemetry indicators, active pump pressure metrics, atmospheric moisture, and water volume balances.
- **Neutral Canvas & Foundations:**
  - Base Viewport Background: `#f8fafc` (Slate 50)
  - Primary Surface / Card Elevated: `#ffffff` (Pure White)
  - Nested Well / Data Container: `#f1f5f9` (Slate 100)
  - Border Hairline & Grid Divider: `#e2e8f0` (Slate 200)
  - Emphasized Border & Inactive Switch: `#cbd5e1` (Slate 300)
- **Typographic Neutral Hierarchy:**
  - Primary Data & Headings: `#0f172a` (Slate 900)
  - Secondary Data & Metric Labels: `#1e293b` (Slate 800)
  - Body Reading & Subtext: `#334155` (Slate 700)
  - Muted Metadata & Units: `#64748b` (Slate 500)

### Telemetry Alert States
- **Alert Amber (`#d97706`):** Threshold breaches (low fertilizer, sub-optimal nitrogen).
- **Critical Crimson (`#dc2626`):** Hardware disconnection, critical drought index, pump stalls.

## Typography

The typographic pairing balances engineering rigor with immediate reading comprehension.

- **Space Grotesk** serves as the headline, metric, and telemetry font. Its technical glyph contours and monospaced-adjacent cadence lend an instrumentation feel to real-time figures, sensor coordinates, soil EC readings, and telemetry IDs.
- **Hanken Grotesk** is deployed across body text, tabular row descriptions, and operational notes. It provides high horizontal tracking legibility, remaining sharp across low-resolution rugged tablets and high-density 4K control monitors.
- Tabular figures (`font-variant-numeric: tabular-nums`) must be enforced on all metric roles (`data-display`, `data-metric`, `code-telemetry`) to prevent layout shifts as telemetry feeds stream.

## Layout & Spacing

The layout utilizes a structured fluid column grid optimized for telemetry multi-splits, chart rows, and dynamic hardware matrices.

### Grid & Breakpoints
- **Mobile (320px – 767px):** 4-column layout, 1rem canvas margin, 0.75rem gutters. Sensor cards span full width; data streams stack linearly.
- **Tablet (768px – 1023px):** 8-column layout, 1.5rem canvas margin, 1rem gutters. Supports side-by-side metric tiles and split telemetry viewports.
- **Desktop (1024px – 1439px):** 12-column layout, 2rem canvas margin, 1.25rem gutters. Allows dual analytics charts and a dedicated side panel for valve actuator controls.
- **Ultra-Wide (1440px+):** 16-column layout with 2rem margins, 1.5rem gutters, maxing out at a 1800px inner frame to maintain eye tracking across dense charts.

### Layout Principles
- **Micro-Gaps (`space-xs`, `space-sm`):** Reserved for metric unit associations (e.g., `24.5` to `°C`), tag labels, and status indicators.
- **Component Padding (`space-md`, `space-lg`):** Standard interior padding for compact hardware status blocks and sensor cards.
- **Structural Grouping (`space-xl`):** Vertical separation between telemetry graphs, field zoning partitions, and pump rack groupings.

## Elevation & Depth

This design system avoids dark or heavy drop shadows, relying on **tonal container layering**, **crisp architectural borders**, and **ambient daylight soft diffusion**.

### Layering Hierarchy
1. **Canvas Level (Ground 0):** Background colored `#f8fafc`. Houses the global layout shell.
2. **Container Level (Ground 1):** Main dashboard cards, sensor widgets, and analytical panels rendered in `#ffffff` with a crisp border: `1px solid #e2e8f0`.
3. **Recessed Wells (Ground -1):** Sub-metric groupings, sensor telemetry logs, and chart plot areas rendered in `#f1f5f9` with an inset perimeter line (`#e2e8f0`).
4. **Hover & Focus Elements:** Border shifts from `#e2e8f0` to `#cbd5e1` accompanied by an ultra-subtle ambient lift: `0 4px 12px -2px rgba(15, 23, 42, 0.05)`.
5. **Overlays & Modals:** Elevated at `0 12px 32px -4px rgba(15, 23, 42, 0.08)`, framed by a crisp border (`#cbd5e1`) and backed by a daylight backdrop blur (`rgba(248, 250, 252, 0.8)` with `backdrop-filter: blur(8px)`).

## Shapes

The design system employs a **Soft (Level 1)** geometric standard. This delivers a clean, modern, and clinical silhouette that matches the technical precision of Space Grotesk.

- **Primary Components (Cards, Panels, Containers):** 0.375rem (6px) border radius.
- **Interactive Controls (Buttons, Inputs, Selectors):** 0.25rem (4px) border radius (`roundedness: 1`).
- **Pill Exceptions (Status Indicators & Badges):** 9999px fully rounded caps used exclusively for live telemetry states (`RUNNING`, `STANDBY`, `OFFLINE`).

## Components

### Buttons
- **Primary (Action/Trigger):** Background `#059669`, text `#ffffff`, border none. Hover state transitions to `#047857`. Focus ring: `2px solid #10b981` offset by `2px`.
- **Secondary (Utility):** Background `#ffffff`, text `#0f172a`, border `1px solid #e2e8f0`. Hover state `#f1f5f9` with border `#cbd5e1`.
- **Critical (Emergency Stop/Purge):** Background `#ffffff`, text `#dc2626`, border `1px solid #fecaca`. Hover state `#fef2f2`.

### Telemetry Cards & Metric Blocks
- Structured on `#ffffff` surfaces with a 1px border (`#e2e8f0`).
- Top section holds a high-contrast label (`Space Grotesk`, `label-sm`, uppercase `#64748b`) paired with a live status dot.
- Main value styled in `data-display` (`#0f172a`), followed immediately by the measurement unit in `#64748b`.
- Footer well rendered with `#f1f5f9` background, housing sparklines or micro-deltas (+2.4% vs optimal moisture).

### Chips & Telemetry Status Tags
- **Online / Irrigating:** Background `#ecfdf5`, border `1px solid #a7f3d0`, text `#065f46`, prepended by a pulsating `#10b981` indicator.
- **Standby:** Background `#f1f5f9`, border `1px solid #e2e8f0`, text `#475569`.
- **Warning / Sub-threshold:** Background `#fffbeb`, border `1px solid #fde68a`, text `#92400e`.
- **Fault / Pump Stall:** Background `#fef2f2`, border `1px solid #fecaca`, text `#991b1b`.

### Inputs & Actuator Controls
- **Form Controls & Selectors:** Native `#ffffff` surface, 36px height for data density, border `1px solid #cbd5e1`, text `#0f172a`. Inner placeholder `#94a3b8`.
- **Toggle Switches (Valves & Automation):** 
  - Track: Width 36px, height 20px, `#cbd5e1` when disabled; `#059669` when active.
  - Thumb: Solid white circle, 16px diameter, smooth slide transition.

### Checkboxes & Radio Selection
- Checkbox square with 3px radius. Radio circular.
- Unchecked: `1px solid #cbd5e1`, background `#ffffff`.
- Checked: Background `#059669`, border `#059669`, checkmark icon in `#ffffff`.

### Data Lists & Hardware Tables
- Header: `#f8fafc`, typography `Space Grotesk` `label-sm` uppercase, bottom border `1px solid #cbd5e1`.
- Rows: Alternating hover state with `#f8fafc`. Cell borders `1px solid #f1f5f9`.
- Numeric data columns aligned right with monospaced numerical spacing (`tabular-nums`).

### Sensor Vitality Gauge / Chart Plots
- Cartesian charts utilize `#ffffff` container backgrounds.
- Grid lines styled in `#f1f5f9` (horizontal) and absent on vertical axes.
- Axis typography uses `Space Grotesk` `code-telemetry` in `#64748b`.
- Primary telemetry line uses `#059669` (2px stroke weight) with a soft vertical gradient fill transitioning down to `rgba(5, 150, 105, 0.02)`.