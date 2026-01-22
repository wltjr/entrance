#!/bin/bash
# Test entrance logind integration in docker-efl environment
# This simulates the issue wltjr reported in #63

set -e

echo "=== Setting up Docker environment for logind testing ==="

docker run --rm --privileged \
    -v $(pwd):/work \
    -w /work \
    --tmpfs /run:exec \
    --tmpfs /tmp:exec \
    wltjr/docker-efl bash -c '

# Setup D-Bus system bus (required for logind)
echo "Starting D-Bus system bus..."
mkdir -p /run/dbus
dbus-daemon --system --fork

# Setup systemd-logind requirements
echo "Setting up logind environment..."
mkdir -p /run/systemd/system
mkdir -p /run/systemd/seats
mkdir -p /run/systemd/users
mkdir -p /run/user/1000
chown 1000:1000 /run/user/1000

# Start systemd-logind
echo "Starting systemd-logind..."
/usr/lib/systemd/systemd-logind &
LOGIND_PID=$!
sleep 3

# Check if logind is running
if ! kill -0 $LOGIND_PID 2>/dev/null; then
    echo "ERROR: systemd-logind failed to start"
    exit 1
fi

echo
echo "=== Logind Status ==="
loginctl --version
echo
loginctl list-sessions || echo "No sessions yet (expected)"

echo
echo "=== Installing entrance ==="
cd /work/build_docker
ninja install

echo
echo "=== Creating test user ==="
useradd -m -s /bin/bash testuser || true

echo
echo "=== Simulating entrance session startup (without X) ==="
echo "This will test if PAM + logind session registration works..."

# Run a simple PAM test to see if logind picks it up
echo "Testing PAM authentication and session creation..."
su - testuser -c "echo PAM session test successful" || echo "PAM test failed"

echo
echo "=== Final session check ==="
loginctl list-sessions

echo
echo "=== Check if sessions have seat/tty ==="
loginctl list-sessions --no-legend | while read session rest; do
    if [ -n "$session" ]; then
        echo "Session $session details:"
        loginctl show-session "$session" | grep -E "^(Seat|TTY|VTNr|Type|Class)="
        echo
    fi
done

echo "=== Test complete ==="
kill $LOGIND_PID 2>/dev/null || true
'
