import pytest

from core import check_screenshot


@pytest.mark.timeout(30)
@pytest.mark.skip()
def test_bully():
    check_screenshot(
        "test_roms/bully/bully.gb",
        "test_roms/bully/bully.png",
        "bully",
    )
