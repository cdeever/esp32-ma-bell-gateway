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
| [`wiring/`](wiring/) | Living pages: how each board is wired right now, pin to pin. Edited as the build changes; git keeps the history. |
| [`log/`](log/) | Dated entries, one per bench session or test: `YYYY-MM-DD-short-name.md`, from [`log/_template.md`](log/_template.md). Never tidied afterward. |
| [`results/`](results/) | Raw evidence the log entries link to: CSVs, scope screenshots, photos, audio clips, named after their entry. |

The build guides in [`impl/`](../impl/) are the plan for each build; the notebook records what
happened when it was built.

## Wiring

- [Sub-Assembly A: ESP32 + PCM5100 DAC + PCM1808 ADC](wiring/sub-a-codecs.md)

## Log

Newest first.

- *No entries yet.*
