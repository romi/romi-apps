from pathlib import Path
import argparse
from PIL import Image, ExifTags
import json


def get_timestamp(path):
    USER_COMMENT = 37510  # 0x9286
    with Image.open(path) as im:
        exif = im.getexif()
        if not exif:
            return None

        # Pillow stores many EXIF-subIFD tags here:
        exif_ifd = exif.get_ifd(ExifTags.IFD.Exif)  # requires relatively recent Pillow
        uc = exif_ifd.get(USER_COMMENT)
        return float(uc[8:-1].decode("utf-8"))

    
def split_mjpeg_to_jpegs(mjpeg_path, out_dir):
    """
    Split an MJPEG bytestream (concatenated JPEG frames) into individual JPEG files.

    Assumes the MJPEG file is a raw sequence of JPEG images back-to-back.
    It finds JPEG SOI/EOI markers (FFD8 ... FFD9) and writes each frame as:
        frame_000000.jpg, frame_000001.jpg, ...

    Returns the number of frames written.
    """
    mjpeg_path = Path(mjpeg_path)
    out_dir = Path(out_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    data = mjpeg_path.read_bytes()
    n = len(data)

    SOI = b"\xff\xd8"  # Start Of Image
    EOI = b"\xff\xd9"  # End Of Image

    elements = []
    i = 0
    frames = 0
    while True:
        # Find start of next JPEG
        soi = data.find(SOI, i)
        if soi == -1:
            break

        # Find end of that JPEG (EOI after SOI)
        eoi = data.find(EOI, soi + 2)
        if eoi == -1:
            # Truncated last frame; stop.
            break

        frame_bytes = data[soi : eoi + 2]
        filename = f"frame-{frames:06d}.jpg"
        out_path = out_dir / filename
        out_path.write_bytes(frame_bytes)

        timestamp = get_timestamp(out_path)
        entry = {'file': filename, 'timestamp': timestamp}
        elements.append(entry)
        frames += 1
        i = eoi + 2  # Continue after this JPEG

    return elements


def _build_argparser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description = ("Split a raw MJPEG bytestream (concatenated JPEG frames) "
                       + "into individual .jpg files."))
    p.add_argument("mjpeg", type=Path,
                   help="Path to input .mjpg/.mjpeg file (raw concatenated JPEGs)")
    p.add_argument("out_dir", type=Path,
                   help="Output directory for extracted JPEG frames")
    return p
    

if __name__ == "__main__":
    args = _build_argparser().parse_args()
    data = split_mjpeg_to_jpegs(args.mjpeg, args.out_dir)
    meta_path = Path(args.out_dir) / "meta.json"
    with open(meta_path, 'w') as f:
        json.dump(data, f)
    print(f"Wrote {len(data)} JPEG frames to directory '{args.out_dir}'")
