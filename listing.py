import os

# File extensions to include
extensions = {'.ino', '.cpp', '.h'}

# Output file
out_filename = 'listing.txt'

def should_include(filename):
    return os.path.splitext(filename)[1].lower() in extensions

with open(out_filename, 'w', encoding='utf-8') as out:
    for root, dirs, files in os.walk('.'):
        for file in files:
            if should_include(file):
                filepath = os.path.join(root, file)
                out.write(f'=== {filepath} ===\n')
                try:
                    with open(filepath, 'r', encoding='utf-8', errors='replace') as infile:
                        out.write(infile.read())
                except Exception as e:
                    out.write(f'[Error reading file: {e}]\n')
                out.write('\n\n')
print(f"Done! Listing written to {out_filename}")