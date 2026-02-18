#!/usr/bin/env python3
"""Analyze flash memory usage from AVR map file and ELF file."""

import subprocess
import sys
import re
from pathlib import Path
from collections import defaultdict

def parse_map_file(map_file):
    """Parse .map file to extract memory sections."""
    if not Path(map_file).exists():
        print(f"Map file not found: {map_file}")
        return None
    
    with open(map_file, 'r') as f:
        content = f.read()
    
    # Extract memory configuration
    memory_section = re.search(r'Memory Configuration(.*?)Linker script', content, re.DOTALL)
    if memory_section:
        print("=" * 80)
        print("MEMORY CONFIGURATION")
        print("=" * 80)
        print(memory_section.group(1).strip())
        print()
    
    # Extract memory map
    memory_map = re.search(r'\.text\s+0x[0-9a-f]+\s+0x[0-9a-f]+(.*?)^\.', content, re.DOTALL | re.MULTILINE)
    
    return content

def analyze_elf(elf_file):
    """Use avr-size and avr-nm to analyze ELF file."""
    if not Path(elf_file).exists():
        print(f"ELF file not found: {elf_file}")
        return
    
    print("=" * 80)
    print("SIZE ANALYSIS")
    print("=" * 80)
    
    # Get overall size
    try:
        result = subprocess.run(
            ['avr-size', '-C', '--mcu=atmega328p', elf_file],
            capture_output=True, text=True, check=True
        )
        print(result.stdout)
    except (subprocess.CalledProcessError, FileNotFoundError):
        try:
            # Try without --mcu flag
            result = subprocess.run(
                ['avr-size', '-A', elf_file],
                capture_output=True, text=True, check=True
            )
            print(result.stdout)
        except Exception as e:
            print(f"Could not run avr-size: {e}")
    
    print("\n" + "=" * 80)
    print("TOP FLASH CONSUMERS BY SYMBOL")
    print("=" * 80)
    
    # Get symbol sizes
    try:
        result = subprocess.run(
            ['avr-nm', '--print-size', '--size-sort', '--radix=d', elf_file],
            capture_output=True, text=True, check=True
        )
        
        symbols = []
        for line in result.stdout.strip().split('\n'):
            parts = line.split()
            if len(parts) >= 4:
                try:
                    size = int(parts[1])
                    symbol_type = parts[2]
                    name = ' '.join(parts[3:])
                    if symbol_type in ['T', 't', 'W', 'w']:  # Text/code symbols
                        symbols.append((size, name))
                except ValueError:
                    continue
        
        # Sort by size and print top consumers
        symbols.sort(reverse=True)
        total = sum(s[0] for s in symbols)
        
        print(f"\n{'Size (bytes)':<15} {'% of Total':<12} {'Symbol'}")
        print("-" * 80)
        
        for size, name in symbols[:50]:  # Top 50
            percent = (size / total * 100) if total > 0 else 0
            print(f"{size:<15} {percent:>6.2f}%      {name}")
        
        print(f"\nTotal code size: {total} bytes")
        
        # Group by class/namespace
        print("\n" + "=" * 80)
        print("SIZE BY CLASS/NAMESPACE")
        print("=" * 80)
        
        class_sizes = defaultdict(int)
        for size, name in symbols:
            # Extract class name from C++ mangled names
            if '::' in name:
                class_name = name.split('::')[0]
                class_sizes[class_name] += size
            elif name.startswith('_Z'):  # Mangled C++ name
                # Try to demangle
                try:
                    demangle_result = subprocess.run(
                        ['c++filt', name],
                        capture_output=True, text=True, check=True
                    )
                    demangled = demangle_result.stdout.strip()
                    if '::' in demangled:
                        class_name = demangled.split('::')[0].split('(')[0].strip()
                        class_sizes[class_name] += size
                except:
                    pass
        
        sorted_classes = sorted(class_sizes.items(), key=lambda x: x[1], reverse=True)
        print(f"\n{'Size (bytes)':<15} {'Class/Namespace'}")
        print("-" * 80)
        for class_name, size in sorted_classes[:30]:
            print(f"{size:<15} {class_name}")
            
    except Exception as e:
        print(f"Could not analyze symbols: {e}")

if __name__ == '__main__':
    build_dir = Path(r'C:\Users\Chris\Documents\PlatformIO\Projects\BinaryClock_ESP32\.pio\build\UNO_R3')
    
    elf_file = build_dir / 'firmware.elf'
    map_file = build_dir / 'firmware.map'
    
    if elf_file.exists():
        analyze_elf(str(elf_file))
    
    if map_file.exists():
        parse_map_file(str(map_file))
    else:
        print(f"\nNo files found. Please build first with: pio run -e UNO_R3")
