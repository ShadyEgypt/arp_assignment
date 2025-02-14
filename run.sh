cd ./Binary

# Launch server and wait until it's ready
konsole --new-tab -e ./server &
echo "Starting server..."
sleep 5  # Wait longer initially for the server to stabilize

# Function to check server readiness - modify according to your actual check
server_ready() {
    # This could be a curl command to a health check endpoint, for example:
    # curl http://localhost:8080/health -o /dev/null -s -I | grep "200 OK"
    pgrep -x server > /dev/null  # Example: check if process named 'server' is running
}

# Wait for server to be ready
until server_ready; do
    echo "Waiting for server to be ready..."
    sleep 2
done
echo "Server is ready."

# Continue with other processes
konsole --new-tab -e ./display &
sleep 2

konsole --new-tab -e ./map &
sleep 2

konsole --new-tab -e ./obstacles &
sleep 2

konsole --new-tab -e ./targets &
sleep 2

konsole --new-tab -e ./drone &
sleep 2

konsole --new-tab -e ./watchdog &
echo "All binaries are running in separate Konsole tabs."
