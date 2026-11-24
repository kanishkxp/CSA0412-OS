#!/usr/bin/env bash
# Firewall: allow only SSH, HTTP and HTTPS. Run as root.
ufw default deny incoming
ufw default allow outgoing
ufw allow 22/tcp      # SSH
ufw allow 80/tcp      # HTTP (redirects to HTTPS)
ufw allow 443/tcp     # HTTPS
ufw --force enable
ufw status verbose

# SSH with keys only (edit /etc/ssh/sshd_config, then: systemctl restart ssh)
#   PasswordAuthentication no
#   PermitRootLogin no
#   PubkeyAuthentication yes
# On my laptop: ssh-keygen -t ed25519 ; ssh-copy-id admin@server
