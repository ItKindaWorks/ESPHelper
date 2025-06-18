import os
import subprocess
import shutil
import minify_html


    
STATIC_DIR = "static"
OUTPUT_DIR = "src"

def convert_file(source_path, dest_path):
    if source_path.endswith(".html"):
        print(f"\t[HTML->H] Minifying {source_path} to {dest_path}")
        temp_path = source_path + ".tmp"  
        shutil.move(source_path, temp_path)  
        

        with open(temp_path, "r") as temp_file:
            content = temp_file.read()
            minified = minify_html.minify(content, minify_js=False, remove_processing_instructions=True)

            with open(source_path, "w") as minified_file:
                minified_file.write(minified)

        with open(dest_path, "w") as out_file:
            subprocess.run(["xxd", "-i", source_path], stdout=out_file)
        
        os.remove(source_path)  # Clean up the temporary file
        shutil.move(temp_path, source_path) # Restore the original file name


    else:
        with open(dest_path, "w") as out_file:
            subprocess.run(["xxd", "-i", source_path], stdout=out_file)
            
    
        
def post_process_file(file_path):
    # Post-process to add PROGMEM to the array declaration
    with open(file_path, "r") as file:
        lines = file.readlines()
    
    lines = ["#pragma once\n\n"] + lines  # Add pragma once at the top

    with open(file_path, "w") as file:
        for line in lines:
            if "[] =" in line:
                line = line.replace("[] =", "[] PROGMEM =")
            if "unsigned" in line:
                line = line.replace("unsigned", "inline unsigned")
            file.write(line)


def main():

    # print("[HTML->H] Converting HTML/CSS/JS files to C header files...")
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)
    for filename in os.listdir(STATIC_DIR):
        if filename.endswith(".html") or filename.endswith(".css") or filename.endswith(".js"):
            input_file = os.path.join(STATIC_DIR, filename)
            output_file = os.path.join(OUTPUT_DIR, filename.replace('.', '_') + ".h")
            print(f"[HTML->H] Converting {input_file} -> {output_file}")
            convert_file(input_file, output_file)
            post_process_file(output_file)

if __name__ == "__main__":
    main()
