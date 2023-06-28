import hashlib
import argparse
'''
Usage: checksumValidate.py -f <filename> -r <filename>
Defaults: -f filename_decompression.raw -r filename.raw

Result:
    -   Shows if the files passed have the same Checksum or not
'''

def calculate_checksum(file_path):
    """
    Calculates the checksum (SHA-256) of a file.
    """
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as file:
        for chunk in iter(lambda: file.read(4096), b""):
            sha256_hash.update(chunk)
    return sha256_hash.hexdigest()


def validate_checksum(file1_path, file2_path):
    """
    Validates the checksum of two binary files.
    """
    checksum1 = calculate_checksum(file1_path)
    checksum2 = calculate_checksum(file2_path)

    if checksum1 == checksum2:
        print("Checksums match. The files are identical.")
    else:
        print("Checksums do not match. The files differ.")

if __name__=="__main__":
    parser = argparse.ArgumentParser(
                        prog='checkSumValidate.py',
                        description='Given two files verify their checksums to detect if any bits have changed',
                        epilog='Use this only for validation of decompressed files ')
    parser.add_argument("-f","--file",help="decompressed file that is to be validated",default="/home/osboxes/faser-daq/gsoc_data/Faser-Physics-009015-00015_decompressed.raw")
    parser.add_argument("-r","--ref",help="decompressed file that is to be validated",default="/home/osboxes/faser-daq/gsoc_data/Faser-Physics-009015-00015_ref.raw")
    args  = parser.parse_args()
    file1_path = args.file
    file2_path = args.ref

    validate_checksum(file1_path, file2_path)