# Localization System

This directory contains YAML localization strings parsed dynamically at startup by the C++20 `LocalizationManager` via `yaml-cpp`.

## Directory Structure
* `locales/*.yml`: Language translation dictionaries (`en.yml`, `hi.yml`, `ar.yml`, `tr.yml`).
* `src/locales.cpp`: C++20 parsing engine that indexes message templates and provides runtime string interpolation.

## Adding a New Language
1. Create `locales/{code}.yml` (e.g., `es.yml`).
2. Define language name header:
   ```yaml
   name: "Español"
   ```
3. Copy translation keys from `en.yml` and provide translations.
4. Restart the bot engine; new locales are automatically detected and loaded on boot.

## Supported Locales

| Language | Code |
| :--- | :--- |
| English | `en` |
| Hindi | `hi` |
| Arabic | `ar` |
| Turkish | `tr` |