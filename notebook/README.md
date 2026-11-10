# Engineering Notebook

The working notebook for building the Ma Bell Gateway: wiring as actually built, bench logs, test
results, scope captures and dead ends. It is written at the bench, kept in the open, and **not
reviewed documentation**. Entries can be rough, wrong or overtaken by later ones.

The settled design lives on the [project site](https://cdeever.github.io/esp32-ma-bell-gateway/)
(Sphinx, `docs/`). When something here is settled, such as a measured result or a decision, it is
written up properly there, with a link back to the notebook entry as its evidence. The plan for
what to build next is the [MaBell project plan](https://deevnet.github.io/deevnet-docs/docs/roadmap/mabell/plan/).

## Layout

| Folder | What goes in it |
|---|---|
| [`build-guides/`](build-guides/) | The plan for each build: parts, wiring order, test matrices. Written before a build; corrected when the bench proves them wrong. |
| [`wiring/`](wiring/) | Living pages: how each board is wired right now, pin to pin. Edited as the build changes; git keeps the history. |
| [`log/`](log/) | Dated entries, one per bench session or test: `YYYY-MM-DD-short-name.md`, from [`log/_template.md`](log/_template.md). Never tidied afterward. |
| [`results/`](results/) | Raw evidence the log entries link to: CSVs, scope screenshots, photos, audio clips, named after their entry. |

## Build guides

One per prototype stage of the [project plan](https://deevnet.github.io/deevnet-docs/docs/roadmap/mabell/plan/):

- [Prototyping build guide](build-guides/prototyping-build-guide.md): Sub-Assembly A (ESP32 + codecs) is the base for every version. Its Sub-B and Sub-C are the HC-5504B path, optional in v3.
- [Low-voltage build](build-guides/low-voltage-test-rig.md): v2, a corded phone on a 5V loop. Its appendix stand-ins also serve v1.
- [SLIC module build](build-guides/slic-module-build-guide.md): v3, the AG1171/KS0835F module and a real bell.

## Wiring

- [Sub-Assembly A: ESP32 + PCM5100 DAC + PCM1808 ADC](wiring/sub-a-codecs.md)
- [Bench rig: ESP32 + MAX98357A I2S amp + hook switch](wiring/max98357a-tone-test.md)

## Log

Newest first.

- [2026-11-10: Call tones, speaker ring and hook switch on the MAX98357A rig](log/2026-11-10-tones-ring-hook.md)
