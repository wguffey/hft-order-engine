#!/usr/bin/env python3
"""
Generate PNG diagrams from PlantUML files.
"""

import os
import plantuml
import glob


def main():
    """Generate PNG diagrams from PlantUML files."""
    # Create output directory if it doesn't exist
    os.makedirs("docs/images", exist_ok=True)

    # Find all PlantUML files
    puml_files = glob.glob("docs/architecture/*.puml")

    # Initialize PlantUML renderer
    plantuml_url = "http://www.plantuml.com/plantuml/img/"
    renderer = plantuml.PlantUML(url=plantuml_url)

    print(f"Found {len(puml_files)} PlantUML files")

    # Generate PNG for each PlantUML file
    for puml_file in puml_files:
        base_name = os.path.basename(puml_file)
        output_name = os.path.splitext(base_name)[0] + ".png"
        output_path = os.path.join("docs/images", output_name)

        print(f"Generating {output_path} from {puml_file}")

        # Generate PNG
        renderer.processes_file(puml_file, outfile=output_path)

        print(f"Generated {output_path}")

    print("All diagrams generated successfully!")


if __name__ == "__main__":
    main()
