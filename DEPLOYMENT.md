# Deployment Guide: CSI Logging on POWDER

## Overview

This profile automatically deploys CSI logging scripts to your experimental node when you instantiate the experiment on POWDER. No manual upload is needed!

## Automatic Deployment

When you deploy this profile on POWDER:

1. **The profile repository is cloned** to `/local/repository/` on the experimental node
2. **Ansible automatically copies** all CSI logging scripts to `/opt/csi_logging/`
3. **A symlink is created** at `~/csi_logging/` for easy access
4. **CSI logs directory** is created at `/tmp/csi_logs/`

## Accessing the Scripts

After your experiment is deployed, the scripts are available at:

- **`/opt/csi_logging/`** - Main location (system-wide)
- **`~/csi_logging/`** - Symlink in your home directory (easiest to use)
- **`/local/repository/scripts/`** - Original profile repository location

## Quick Start After Deployment

1. **SSH into your experimental node**

2. **Verify scripts are available:**
   ```bash
   ls -la ~/csi_logging/
   ```

3. **Find SRS processing code:**
   ```bash
   ~/csi_logging/find_srs_csi_estimation.sh /opt/srsRAN_Project
   ```

4. **Follow the integration guide:**
   ```bash
   cat ~/csi_logging/INTEGRATION_GUIDE.md
   ```

## Manual Alternative (If Needed)

If for some reason the automatic deployment didn't work, you can manually copy files:

```bash
# The repository is always available at /local/repository
cp -r /local/repository/scripts /opt/csi_logging
chmod +x /opt/csi_logging/*.sh /opt/csi_logging/*.py
```

## What Gets Deployed

The following files are automatically copied to `/opt/csi_logging/`:

- `find_srs_csi_estimation.sh` - Locates SRS code
- `add_csi_logging.sh` - Setup instructions generator
- `csi_logger.h` - CSI logger header
- `csi_logger.cpp` - CSI logger implementation
- `extract_csi.py` - Analysis tool
- `INTEGRATION_GUIDE.md` - Integration instructions
- `CSI_LOGGING_README.md` - Full documentation
- `QUICK_START.md` - Quick reference

## Next Steps

1. Use the scripts to locate and modify the SRS processing code
2. Rebuild srsRAN with CSI logging enabled
3. Run your simulation and collect CSI data
4. Analyze the partial CSI for your extrapolation research

See `QUICK_START.md` for detailed instructions.

