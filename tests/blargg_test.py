import os

import pytest

from core import check_stream

cpu_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/cpu_instrs/")
    for f in files
    if f.endswith(".gb") and f != "cpu_instrs.gb"
]
audio_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/dmg_sound/")
    for f in files
    if f.endswith(".gb")
]
cpu_time_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/instr_timing/")
    for f in files
    if f.endswith(".gb")
]
mem_time_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/mem_timing/")
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(120)
@pytest.mark.parametrize("rom_path", cpu_roms)
def test_blargg_cpu(rom_path):
    check_stream(rom_path, "blargg_cpu")


@pytest.mark.skip()
@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", audio_roms)
def test_blargg_audio(rom_path):
    check_stream(rom_path, "blargg_audio")


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", cpu_time_roms)
def test_blargg_cpu_time(rom_path):
    check_stream(rom_path, "blargg_cpu_time")


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", mem_time_roms)
def test_blargg_mem_time(rom_path):
    check_stream(rom_path, "blargg_mem_time")
