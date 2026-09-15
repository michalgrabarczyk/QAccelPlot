#
# SPDX-FileCopyrightText: 2026 Michal Grabarczyk
# SPDX-License-Identifier: GPL-3.0-only WITH Universal-FOSS-exception-1.0
#
# This file is also available under a separate commercial license.
# See COMMERCIAL-LICENSING.md for contact information.
#
"""Naming convention shared by visual acceptance scripts.

A scenario is one example captured in one state, such as a tab. Its contract lives at
``contracts/<example>/<scenario>.json`` and its screenshot is named
``<example>__<scenario>.png``. Examples without pages use the ``default`` scenario.
Every name is derived from these paths, so no script keeps its own list of examples.
"""

from __future__ import annotations

from dataclasses import dataclass
import json
from pathlib import Path


SCREENSHOT_SEPARATOR = "__"
DEFAULT_CONTRACTS_DIR = Path(__file__).resolve().parent / "contracts"


class ScenarioError(RuntimeError):
    """A contract file does not follow the scenario convention."""


@dataclass(frozen=True)
class Scenario:
    example: str
    name: str
    contract_path: Path
    ai_inspection: bool
    sources: tuple[str, ...]

    @property
    def identity(self) -> str:
        return f"{self.example}/{self.name}"

    @property
    def screenshot_stem(self) -> str:
        return f"{self.example}{SCREENSHOT_SEPARATOR}{self.name}"


def discover_scenarios(contracts_dir: Path = DEFAULT_CONTRACTS_DIR) -> dict[str, Scenario]:
    """Returns every scenario with a contract, keyed by ``<example>/<scenario>``."""
    scenarios: dict[str, Scenario] = {}
    for contract_path in sorted(contracts_dir.glob("*/*.json")):
        try:
            contract = json.loads(contract_path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as error:
            raise ScenarioError(f"Unable to read visual contract {contract_path}: {error}") from error

        ai_inspection = contract.get("ai_inspection", True)
        sources = contract.get("sources", [])
        if not isinstance(ai_inspection, bool):
            raise ScenarioError(f"{contract_path}: ai_inspection must be a boolean.")
        if not isinstance(sources, list) or not all(isinstance(source, str) for source in sources):
            raise ScenarioError(f"{contract_path}: sources must be a list of example-relative paths.")

        scenario = Scenario(
            example=contract_path.parent.name,
            name=contract_path.stem,
            contract_path=contract_path,
            ai_inspection=ai_inspection,
            sources=tuple(sources),
        )
        scenarios[scenario.identity] = scenario
    return scenarios


def scenario_identity_from_screenshot(image_path: Path) -> str | None:
    """Maps ``<example>__<scenario>.png`` to ``<example>/<scenario>``."""
    example, separator, name = image_path.stem.partition(SCREENSHOT_SEPARATOR)
    if not separator or not example or not name:
        return None
    return f"{example}/{name}"
