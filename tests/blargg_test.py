import os

import pytest

from core import check_stream

cpu_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/cpu_instrs/")
    for f in files
    if f.endswith(".gb")
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
cgb_sound_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/cgb_sound/")
    for f in files
    if f.endswith(".gb")
]
halt_bug_roms = ["test_roms/blargg/halt_bug.gb"]
intr_time_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/interrupt_time/")
    for f in files
    if f.endswith(".gb")
]
mem_time2_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/mem_timing-2/")
    for f in files
    if f.endswith(".gb")
]
oam_bug_roms = [
    os.path.join(d, f)
    for (d, _, files) in os.walk("test_roms/blargg/oam_bug/")
    for f in files
    if f.endswith(".gb")
]


@pytest.mark.timeout(120)
@pytest.mark.parametrize("rom_path", cpu_roms)
def test_blargg_cpu(rom_path):
    check_stream(rom_path, "blargg_cpu")


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


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom_path", halt_bug_roms)
def test_blargg_halt_bug(rom_path):
    check_stream(rom_path, "blargg_halt_bug")


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom_path", intr_time_roms)
def test_blargg_interrupt_time(rom_path):
    check_stream(rom_path, "blargg_interrupt_time")


@pytest.mark.timeout(30)
@pytest.mark.parametrize("rom_path", mem_time2_roms)
def test_blargg_mem_time2(rom_path):
    check_stream(rom_path, "blargg_mem_time2")


@pytest.mark.timeout(60)
@pytest.mark.parametrize("rom_path", oam_bug_roms)
def test_blargg_oam_bug(rom_path):
    check_stream(rom_path, "blargg_oam_bug")


@pytest.mark.timeout(120)
@pytest.mark.parametrize("rom_path", cgb_sound_roms)
def test_blargg_cgb_sound(rom_path):
    check_stream(rom_path, "blargg_cgb_sound")
