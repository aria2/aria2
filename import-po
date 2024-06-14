#!/bin/sh -e

# Update po files using launchpad-export.tar.gz

WORK_DIR=launchpad-work
INPUT_TGZ=$1
PO_DIR=po

if [ -z "$INPUT_TGZ" ]; then
    echo "Usage: import-po /path/to/launchpad-export.tar.gz"
    echo "Specify input launchpad-export.tar.gz file"
    exit 1
fi
if [ ! -e "$INPUT_TGZ" ]; then
    echo "Input file $INPUT_TGZ does not exist"
    exit 1
fi

if [ -e "$WORK_DIR" ]; then
    rm -rf "$WORK_DIR"
fi
mkdir "$WORK_DIR"
echo "Extracting po files from the archive..."
tar -x -C "$WORK_DIR" -f "$INPUT_TGZ"

echo "Renaming po files..."
# The directory structure of launchpad-export.tar.gz is a bit
# strange. It even contains absolute file path. We first gather all
# files in top level directory.
mv "$WORK_DIR"/aria2/*.po "$WORK_DIR"

echo -n "en@quot en@boldquot" > "$PO_DIR"/LINGUAS
for file in "$WORK_DIR"/*.po; do
    # First remove useless '\r' in messages
    sed -i -e 's/\\r//' "$file"
    bn=$(basename "$file")
    bn=${bn#aria2-}
    dst="$PO_DIR"/"$bn"
    # copy file to po directory
    echo "Moving \`$file' to \`$dst'..."
    mv "$file" "$dst"
    # Update LINGUAS here too.
    echo -n " ${bn%.po}" >> "$PO_DIR"/LINGUAS
done

rm -rf "$WORK_DIR"

cd "$PO_DIR"
make update-po
