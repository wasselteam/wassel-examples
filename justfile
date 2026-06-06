bootstrap:
    python3 -m venv python/.venv && \
    source ./python/.venv/bin/activate && \
    pip install componentize-py

    cd javascript && npm install
