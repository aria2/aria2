"""
Tests for the bdist_wheel tag options (--python-tag and --universal)
"""

import sys
import shutil
import pytest
import py.path
import tempfile
import subprocess

SETUP_PY = """\
from setuptools import setup

setup(
    name="Test",
    version="1.0",
    author_email="author@example.com",
    py_modules=["test"],
)
"""

@pytest.fixture
def temp_pkg(request):
    tempdir = tempfile.mkdtemp()
    def fin():
        shutil.rmtree(tempdir)
    request.addfinalizer(fin)
    temppath = py.path.local(tempdir)
    temppath.join('test.py').write('print("Hello, world")')
    temppath.join('setup.py').write(SETUP_PY)
    return temppath

def test_default_tag(temp_pkg):
    subprocess.check_call([sys.executable, 'setup.py', 'bdist_wheel'],
            cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py%s-' % (sys.version[0],))
    assert wheels[0].ext == '.whl'

def test_explicit_tag(temp_pkg):
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel', '--python-tag=py32'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py32-')
    assert wheels[0].ext == '.whl'

def test_universal_tag(temp_pkg):
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel', '--universal'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py2.py3-')
    assert wheels[0].ext == '.whl'

def test_universal_beats_explicit_tag(temp_pkg):
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel', '--universal', '--python-tag=py32'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py2.py3-')
    assert wheels[0].ext == '.whl'

def test_universal_in_setup_cfg(temp_pkg):
    temp_pkg.join('setup.cfg').write('[bdist_wheel]\nuniversal=1')
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py2.py3-')
    assert wheels[0].ext == '.whl'

def test_pythontag_in_setup_cfg(temp_pkg):
    temp_pkg.join('setup.cfg').write('[bdist_wheel]\npython_tag=py32')
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py32-')
    assert wheels[0].ext == '.whl'

def test_legacy_wheel_section_in_setup_cfg(temp_pkg):
    temp_pkg.join('setup.cfg').write('[wheel]\nuniversal=1')
    subprocess.check_call(
        [sys.executable, 'setup.py', 'bdist_wheel'],
        cwd=str(temp_pkg))
    dist_dir = temp_pkg.join('dist')
    assert dist_dir.check(dir=1)
    wheels = dist_dir.listdir()
    assert len(wheels) == 1
    assert wheels[0].basename.startswith('Test-1.0-py2.py3-')
    assert wheels[0].ext == '.whl'

