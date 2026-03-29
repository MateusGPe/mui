#!/usr/bin/env python3
import os

# A lenient map to translate markdown language tags to file extensions
LANGUAGE_EXTENSIONS = {
    'python': '.py', 'py': '.py',
    'javascript': '.js', 'js': '.js',
    'typescript': '.ts', 'ts': '.ts',
    'html': '.html',
    'css': '.css',
    'bash': '.sh', 'sh': '.sh', 'shell': '.sh',
    'json': '.json',
    'yaml': '.yaml', 'yml': '.yml',
    'xml': '.xml',
    'c': '.c',
    'cpp': '.cpp', 'c++': '.cpp',
    'java': '.java',
    'csharp': '.cs', 'cs': '.cs',
    'go': '.go',
    'rust': '.rs', 'rs': '.rs',
    'ruby': '.rb', 'rb': '.rb',
    'php': '.php',
    'sql': '.sql',
    'markdown': '.md', 'md': '.md',
}

def get_extension(language_tag):
    """Safely determine the file extension based on the markdown tag."""
    lang = language_tag.strip().lower()
    
    # Clean up inline modifiers sometimes added by parsers (e.g., 'python title="main.py"')
    lang = lang.split(' ')[0] 
    
    if not lang:
        return '.txt'  # Default for untagged blocks
        
    # Return known extension, or fallback to treating the tag as the extension
    return LANGUAGE_EXTENSIONS.get(lang, f".{lang}")

def extract_code_blocks(input_filepath, output_dir="extracted_code"):
    """Extracts code blocks from a markdown file without using regex."""
    
    # Fail-safe 1: Check if input file exists and is readable
    try:
        with open(input_filepath, 'r', encoding='utf-8') as f:
            lines = f.readlines()
    except FileNotFoundError:
        print(f"[-] Error: File '{input_filepath}' not found.")
        return
    except Exception as e:
        print(f"[-] Error reading file: {e}")
        return

    # Fail-safe 2: Ensure output directory exists
    if not os.path.exists(output_dir):
        try:
            os.makedirs(output_dir)
        except Exception as e:
            print(f"[-] Error creating output directory '{output_dir}': {e}")
            return

    # State machine variables
    in_code_block = False
    current_block = []
    current_language = ""
    delimiter_char = ""
    block_counter = 1

    for line in lines:
        stripped_line = line.lstrip() # Lenient: ignore preceding spaces/tabs

        if not in_code_block:
            # Detect opening delimiter (backticks or tildes)
            if stripped_line.startswith('```') or stripped_line.startswith('~~~'):
                in_code_block = True
                delimiter_char = stripped_line[0] # Track if it's '`' or '~'
                
                # Count how many characters make up the delimiter (e.g., could be 4 backticks)
                idx = 0
                while idx < len(stripped_line) and stripped_line[idx] == delimiter_char:
                    idx += 1
                
                # Extract the language tag (everything after the delimiter)
                current_language = stripped_line[idx:].strip()
                current_block = []
        else:
            # We are inside a block. Check if this line is the closing delimiter.
            # It must start with the same character and have at least 3 of them.
            if stripped_line.startswith(delimiter_char * 3):
                # Verify it's purely a closing delimiter (ignoring trailing whitespace)
                is_closing = True
                for char in stripped_line.rstrip():
                    if char != delimiter_char:
                        is_closing = False
                        break
                
                if is_closing:
                    # Successfully found the end of the block. Save it.
                    ext = get_extension(current_language)
                    filename = f"block_{block_counter:03d}{ext}"
                    filepath = os.path.join(output_dir, filename)
                    
                    try:
                        with open(filepath, 'w', encoding='utf-8') as out_f:
                            out_f.writelines(current_block)
                        print(f"[+] Saved: {filepath} (Type: {current_language or 'None'})")
                        block_counter += 1
                    except Exception as e:
                        print(f"[-] Error saving {filepath}: {e}")
                        
                    # Reset state machine
                    in_code_block = False
                    current_block = []
                    current_language = ""
                    delimiter_char = ""
                    continue
            
            # If it's not the closing delimiter, add the raw line to our block
            current_block.append(line)

    # Fail-safe 3: Handle an unclosed block at the end of the file
    if in_code_block and current_block:
        ext = get_extension(current_language)
        filename = f"block_{block_counter:03d}_unclosed{ext}"
        filepath = os.path.join(output_dir, filename)
        try:
            with open(filepath, 'w', encoding='utf-8') as out_f:
                out_f.writelines(current_block)
            print(f"[!] Saved UNCLOSED block: {filepath} (Type: {current_language or 'None'})")
        except Exception as e:
            print(f"[-] Error saving unclosed block {filepath}: {e}")

# ==========================================
# Example Usage
# ==========================================
if __name__ == "__main__":
    # Create a dummy markdown file to test
    test_md = "mm.md"
    print(f"Processing '{test_md}'...")
    extract_code_blocks(test_md, output_dir="extracted_files")