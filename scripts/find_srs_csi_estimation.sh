#!/bin/bash
# Script to locate SRS channel estimation code in srsRAN Project
# This helps identify where CSI is estimated from SRS signals

set -e

SRSRAN_DIR="${1:-/opt/srsRAN_Project}"

if [ ! -d "$SRSRAN_DIR" ]; then
    echo "Error: srsRAN Project directory not found at $SRSRAN_DIR"
    echo "Usage: $0 [srsran_project_path]"
    exit 1
fi

echo "Searching for SRS channel estimation code in $SRSRAN_DIR"
echo "=================================================="
echo ""

# Search for SRS-related files
echo "1. SRS processor files:"
find "$SRSRAN_DIR" -type f -name "*srs*" -o -name "*SRS*" | grep -E "\.(cpp|h|hpp)$" | head -20

echo ""
echo "2. Channel estimation files:"
find "$SRSRAN_DIR" -type f \( -name "*channel*estimat*" -o -name "*channel_estimat*" \) | grep -E "\.(cpp|h|hpp)$" | head -20

echo ""
echo "3. Files containing 'estimate_channel' or 'srs_channel':"
grep -r "estimate_channel\|srs_channel" "$SRSRAN_DIR/lib" --include="*.cpp" --include="*.h" --include="*.hpp" 2>/dev/null | head -20

echo ""
echo "4. Files containing 'process_srs' or 'handle_srs':"
grep -r "process_srs\|handle_srs" "$SRSRAN_DIR/lib" --include="*.cpp" --include="*.h" --include="*.hpp" 2>/dev/null | head -20

echo ""
echo "5. Most likely locations for SRS processing:"
echo "   - lib/phy/upper/channel_processors/"
echo "   - lib/phy/upper/signal_processors/"
echo "   - lib/phy/upper/"
echo ""
echo "Checking these directories:"
for dir in "lib/phy/upper/channel_processors" "lib/phy/upper/signal_processors" "lib/phy/upper"; do
    if [ -d "$SRSRAN_DIR/$dir" ]; then
        echo "  $dir:"
        find "$SRSRAN_DIR/$dir" -type f -name "*srs*" -o -name "*channel*" | head -10
    fi
done

