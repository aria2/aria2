import wheel.install
import hashlib
try:
    from StringIO import StringIO
except ImportError:
    from io import BytesIO as StringIO
import zipfile
import pytest

def test_verifying_zipfile():
    if not hasattr(zipfile.ZipExtFile, '_update_crc'):
        pytest.skip('No ZIP verification. Missing ZipExtFile._update_crc.')
    
    sio = StringIO()
    zf = zipfile.ZipFile(sio, 'w')
    zf.writestr("one", b"first file")
    zf.writestr("two", b"second file")
    zf.writestr("three", b"third file")
    zf.close()
    
    # In default mode, VerifyingZipFile checks the hash of any read file
    # mentioned with set_expected_hash(). Files not mentioned with
    # set_expected_hash() are not checked.
    vzf = wheel.install.VerifyingZipFile(sio, 'r')
    vzf.set_expected_hash("one", hashlib.sha256(b"first file").digest())
    vzf.set_expected_hash("three", "blurble")
    vzf.open("one").read()
    vzf.open("two").read()
    try:
        vzf.open("three").read()
    except wheel.install.BadWheelFile:
        pass
    else:
        raise Exception("expected exception 'BadWheelFile()'")
    
    # In strict mode, VerifyingZipFile requires every read file to be
    # mentioned with set_expected_hash().
    vzf.strict = True
    try:
        vzf.open("two").read()
    except wheel.install.BadWheelFile:
        pass
    else:
        raise Exception("expected exception 'BadWheelFile()'")
        
    vzf.set_expected_hash("two", None)
    vzf.open("two").read()
    
def test_pop_zipfile():
    sio = StringIO()
    zf = wheel.install.VerifyingZipFile(sio, 'w')
    zf.writestr("one", b"first file")
    zf.writestr("two", b"second file")
    zf.close()
    
    try:
        zf.pop()
    except RuntimeError:
        pass # already closed
    else:
        raise Exception("expected RuntimeError")
    
    zf = wheel.install.VerifyingZipFile(sio, 'a')
    zf.pop()
    zf.close()
    
    zf = wheel.install.VerifyingZipFile(sio, 'r')
    assert len(zf.infolist()) == 1
    