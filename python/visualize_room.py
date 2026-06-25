"""
Screeps Room Blueprint Visualizer
==================================
Usage:
    python visualize_room.py memory.json [roomName]

    - memory.json : file containing the output of JSON.stringify(Memory.rooms)
                    from the Screeps in-game console
    - roomName    : optional, e.g. E28S42. If omitted, uses the first room found.

Controls (interactive window):
    - Hover over a tile to see its details in the title bar
    - Close the window to exit

How to get the memory JSON from Screeps:
    1. Open the in-game console
    2. Run:  JSON.stringify(Memory.rooms)
    3. Copy the output
    4. Paste it into a file called memory.json
"""

import json
import sys
import argparse
import tkinter as tk
from tkinter import messagebox

# ── visual config ──────────────────────────────────────────────────────────────

ROOM_SIZE   = 50        # Screeps rooms are always 50x50
CELL_SIZE   = 28        # pixels per tile
FONT_SIZE   = 14         # font size for cLevel labels

# Colours per structure type
COLORS = {
    "plain":              "#1a1a2e",   # dark background
    "road":               "#4a4e69",   # grey-blue road
    "container":          "#e9c46a",   # yellow
    "link":               "#4cc9f0",   # cyan
    "storage":            "#f77f00",   # orange
    "tower":              "#e63946",   # red
    "extension":          "#a8dadc",   # light teal
    "spawn":              "#ffffff",   # white
    "constructedWall":    "#2d2d2d",   # near-black
    "rampart":            "#2a9d8f",   # teal
    "controller":         "#9b5de5",   # purple (for reference dot)
    "source":             "#ffd166",   # gold (for reference dot)
}

# Short labels shown on tiles
LABELS = {
    "road":               "·",
    "container":          "C",
    "link":               "L",
    "storage":            "St",
    "tower":              "T",
    "extension":          "E",
    "spawn":              "Sp",
    "constructedWall":    "W",
    "rampart":            "R",
    "controller":         "Ctrl",
    "source":             "Src",
}

# cLevel colour bands (darker = lower level requirement)
CLEVEL_COLORS = {
    1: "#264653",
    2: "#287271",
    3: "#2a9d8f",
    4: "#57cc99",
    5: "#80ed99",
    6: "#c7f9cc",
    7: "#ffffb3",
    8: "#ffd166",
}


# ── data loading ───────────────────────────────────────────────────────────────

def load_memory(path: str, room_name: str | None):
    with open(path) as f:
        data = json.load(f)

    # data is either {roomName: {blueprint:...}} or already the room object
    if room_name:
        if room_name not in data:
            raise ValueError(f"Room '{room_name}' not found. Available: {list(data.keys())}")
        room_data = data[room_name]
    else:
        room_name = next(iter(data))
        room_data = data[room_name]
        print(f"No room specified — using '{room_name}'")

    blueprint = room_data.get("blueprint", {})
    return room_name, blueprint


def parse_blueprint(blueprint: dict):
    """Return a dict of (x, y) -> entry for all blueprint entries."""
    tiles = {}
    for key, entry in blueprint.items():
        x = entry.get("x", -1)
        y = entry.get("y", -1)
        if 0 <= x < ROOM_SIZE and 0 <= y < ROOM_SIZE:
            tiles[(x, y)] = entry
    return tiles


# ── rendering ─────────────────────────────────────────────────────────────────

class RoomVisualizer:
    def __init__(self, root: tk.Tk, room_name: str, tiles: dict):
        self.root      = root
        self.room_name = room_name
        self.tiles     = tiles

        canvas_px = ROOM_SIZE * CELL_SIZE
        root.title(f"Screeps Blueprint — {room_name}")
        root.resizable(False, False)

        # Legend panel on the right
        legend_width = 320
        frame = tk.Frame(root)
        frame.pack(fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(
            frame,
            width=canvas_px,
            height=canvas_px,
            bg=COLORS["plain"],
            highlightthickness=0,
        )
        self.canvas.pack(side=tk.LEFT)

        legend_frame = tk.Frame(frame, bg="#0d0d1a", width=legend_width)
        legend_frame.pack(side=tk.LEFT, fill=tk.Y)
        self._build_legend(legend_frame)

        # Status bar at bottom
        self.status = tk.StringVar(value="Hover over a tile for details")
        status_bar = tk.Label(
            root,
            textvariable=self.status,
            bg="#0d0d1a",
            fg="#cccccc",
            anchor="w",
            font=("Courier", 9),
        )
        status_bar.pack(fill=tk.X)

        self._draw_grid()
        self._draw_tiles()

        self.canvas.bind("<Motion>", self._on_hover)

    def _build_legend(self, parent):
        tk.Label(parent, text="Legend", bg="#0d0d1a", fg="white",
                 font=("Courier", 14, "bold")).pack(pady=(8, 4))

        entries = [
            ("road",            "Road"),
            ("container",       "Container"),
            ("link",            "Link"),
            ("storage",         "Storage"),
            ("tower",           "Tower"),
            ("extension",       "Extension"),
            ("spawn",           "Spawn"),
            ("constructedWall", "Wall"),
            ("rampart",         "Rampart"),
        ]
        for stype, label in entries:
            row = tk.Frame(parent, bg="#0d0d1a")
            row.pack(fill=tk.X, padx=6, pady=1)
            swatch = tk.Label(row, bg=COLORS.get(stype, "#888888"),
                              width=2, relief=tk.FLAT)
            swatch.pack(side=tk.LEFT, padx=(0, 6))
            tk.Label(row, text=label, bg="#0d0d1a", fg="white",
                     font=("Courier", 8)).pack(side=tk.LEFT)

        tk.Label(parent, text="\ncLevel tint", bg="#0d0d1a", fg="#aaaaaa",
                 font=("Courier", 8, "italic")).pack()

        for level, color in CLEVEL_COLORS.items():
            row = tk.Frame(parent, bg="#0d0d1a")
            row.pack(fill=tk.X, padx=3, pady=1)
            swatch = tk.Label(row, bg=color, width=6, relief=tk.FLAT)
            swatch.pack(side=tk.LEFT, padx=(0, 6))
            tk.Label(row, text=f"RCL {level}", bg="#0d0d1a", fg="white",
                     font=("Courier", 8)).pack(side=tk.LEFT)

    def _cell_rect(self, x, y):
        x1 = x * CELL_SIZE
        y1 = y * CELL_SIZE
        return x1, y1, x1 + CELL_SIZE, y1 + CELL_SIZE

    def _draw_grid(self):
        """Draw faint grid lines."""
        for i in range(0, ROOM_SIZE * CELL_SIZE, CELL_SIZE * 5):
            self.canvas.create_line(i, 0, i, ROOM_SIZE * CELL_SIZE,
                                    fill="#2a2a3e", width=1)
            self.canvas.create_line(0, i, ROOM_SIZE * CELL_SIZE, i,
                                    fill="#2a2a3e", width=1)

    def _draw_tiles(self):
        for (x, y), entry in self.tiles.items():
            stype  = entry.get("sType", "plain")
            clevel = entry.get("cLevel", 1)
            role   = entry.get("role", "")
            status = entry.get("status", "planned")

            color  = COLORS.get(stype, "#888888")
            x1, y1, x2, y2 = self._cell_rect(x, y)

            # Draw tile background
            self.canvas.create_rectangle(x1, y1, x2, y2,
                                         fill=color, outline="", tags="tile")

            # Draw cLevel tint as a small corner square (bottom-right)
            tint = CLEVEL_COLORS.get(clevel, "#ffffff")
            margin = CELL_SIZE - 10
            self.canvas.create_rectangle(x1 + margin, y1 + margin,
                                         x2, y2,
                                         fill=tint, outline="", tags="tile")

            # Draw label (skip for roads — too small)
            label = LABELS.get(stype, "")
            if label and stype != "road":
                self.canvas.create_text(
                    x1 + CELL_SIZE // 2,
                    y1 + CELL_SIZE // 2,
                    text=label,
                    fill="#0d0d1a",
                    font=("Courier", FONT_SIZE, "bold"),
                    tags="tile",
                    )

            # Dim unbuilt (planned) tiles slightly
            if status == "planned":
                self.canvas.create_rectangle(x1, y1, x2, y2,
                                             fill="", outline="#ffffff",
                                             width=1, dash=(2, 3), tags="tile")

    def _on_hover(self, event):
        x = event.x // CELL_SIZE
        y = event.y // CELL_SIZE
        if 0 <= x < ROOM_SIZE and 0 <= y < ROOM_SIZE:
            entry = self.tiles.get((x, y))
            if entry:
                stype  = entry.get("sType", "?")
                clevel = entry.get("cLevel", "?")
                role   = entry.get("role", "-")
                status = entry.get("status", "?")
                self.status.set(
                    f"({x:2d},{y:2d})  type={stype:<20} RCL={clevel}  "
                    f"role={role:<10} status={status}"
                )
            else:
                self.status.set(f"({x:2d},{y:2d})  plain")


# ── entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(description="Screeps Room Blueprint Visualizer")
    parser.add_argument("memory_file", help="Path to memory JSON file")
    parser.add_argument("room_name", nargs="?", default=None,
                        help="Room name, e.g. E28S42 (optional, uses first room if omitted)")
    args = parser.parse_args()

    try:
        room_name, blueprint = load_memory(args.memory_file, args.room_name)
    except (FileNotFoundError, json.JSONDecodeError, ValueError) as e:
        print(f"Error: {e}")
        sys.exit(1)

    tiles = parse_blueprint(blueprint)
    print(f"Loaded {len(tiles)} blueprint entries for room '{room_name}'")

    root = tk.Tk()
    RoomVisualizer(root, room_name, tiles)
    root.mainloop()


if __name__ == "__main__":
    main()