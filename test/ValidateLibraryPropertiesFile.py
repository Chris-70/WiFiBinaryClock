#!/usr/bin/env python3
"""
Arduino library.properties Validator
Validates syntax, required fields, and dependency names against Arduino Library Manager.
"""

import re
import sys
import gzip
import json
import urllib.request
from pathlib import Path

# Allowed categories from Arduino specification
ALLOWED_CATEGORIES = {
    "Communication", "Data Processing", "Data Storage", "Device Control",
    "Display", "Other", "Sensors", "Signal Input/Output", "Timing", "Uncategorized"
}

# Required fields for Arduino Library Manager
REQUIRED_FIELDS = [
    "name", "version", "author", "maintainer",
    "sentence", "paragraph", "category", "url", "architectures"
]

# Regex for semantic versioning: MAJOR.MINOR.PATCH
SEMVER_REGEX = re.compile(r"^\d+\.\d+\.\d+$")

# Arduino Library Index URL
LIBRARY_INDEX_URL = "https://downloads.arduino.cc/libraries/library_index.json.gz"

def parse_properties(file_path):
    """Parses a .properties file into a dictionary."""
    props = {}
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            for line in f:
                line = line.strip()
                if not line or line.startswith("#"):
                    continue
                if "=" not in line:
                    print(f"⚠️  Invalid line (no '='): {line}")
                    continue
                key, value = line.split("=", 1)
                props[key.strip()] = value.strip()
    except FileNotFoundError:
        print(f"❌ File not found: {file_path}")
        sys.exit(1)
    return props

def fetch_library_index():
    """Downloads and parses the Arduino Library Manager index."""
    try:
        print("📥 Downloading Arduino Library Index...")
        with urllib.request.urlopen(LIBRARY_INDEX_URL) as response:
            compressed_data = response.read()
        data = gzip.decompress(compressed_data).decode("utf-8")
        index = json.loads(data)
        # Extract all valid library names
        return {lib["name"] for lib in index.get("libraries", [])}
    except Exception as e:
        print(f"⚠️  Could not fetch library index: {e}")
        return set()

def validate_properties(props, valid_lib_names):
    """Validates the parsed properties."""
    errors = []
    warnings = []

    # Check required fields
    for field in REQUIRED_FIELDS:
        if field not in props or not props[field]:
            errors.append(f"Missing required field: {field}")

    # Validate version format
    version = props.get("version", "")
    if version and not SEMVER_REGEX.match(version):
        errors.append(f"Invalid version format '{version}'. Must be MAJOR.MINOR.PATCH (e.g., 1.0.0)")

    # Validate category
    category = props.get("category", "")
    if category and category not in ALLOWED_CATEGORIES:
        errors.append(f"Invalid category '{category}'. Must be one of: {', '.join(sorted(ALLOWED_CATEGORIES))}")

    # Validate includes syntax
    includes = props.get("includes", "")
    if includes:
        for inc in includes.split(","):
            inc = inc.strip()
            if not inc.endswith(".h"):
                warnings.append(f"Include '{inc}' does not end with .h")
            if " " in inc:
                warnings.append(f"Include '{inc}' contains spaces — not recommended")

    # Validate depends syntax and names
    depends = props.get("depends", "")
    if depends:
        for dep in depends.split(","):
            dep = dep.strip()
            if not dep:
                warnings.append("Empty dependency name found in 'depends' list")
                continue
            if valid_lib_names and dep not in valid_lib_names:
                warnings.append(f"Dependency '{dep}' not found in Arduino Library Manager index. "
                                f"Check spelling and use the exact 'name' from the library.properties of that library.")

    return errors, warnings

def main():
    if len(sys.argv) != 2:
        print("Usage: python validate_library_properties.py <path_to_library.properties>")
        sys.exit(1)

    file_path = Path(sys.argv[1])
    props = parse_properties(file_path)

    # Fetch Arduino Library Manager index
    valid_lib_names = fetch_library_index()

    errors, warnings = validate_properties(props, valid_lib_names)

    print("\nValidation Results:")
    if errors:
        print("\n❌ Errors:")
        for e in errors:
            print(f"  - {e}")
    else:
        print("\n✅ No critical errors found.")

    if warnings:
        print("\n⚠️  Warnings:")
        for w in warnings:
            print(f"  - {w}")
    else:
        print("\n✅ No warnings.")

    if errors:
        sys.exit(1)  # Fail if errors found

if __name__ == "__main__":
    main()
