# LSB Image Steganography Project Documentation

## Project Overview
The LSB (Least Significant Bit) Image Steganography project involves the art of hiding private or sensitive information within digital images. This technique exploits human perception, as the hidden information is imperceptible to the naked eye. The project focuses on embedding messages within .bmp image files using command-line operations.

---

## Features
The application supports the following functionalities:

### Mandatory Features
- **Image and Message Handling:**
  - Accept an image file (e.g., `.bmp`) and a text file containing the message to be hidden.

- **Message Size Analysis:**
  - Analyze the size of the message file to ensure it can fit within the provided image.

- **Magic String Option:**
  - Provide an option to steg a magic string, useful for identifying whether the image has been stegged or not.

- **Decryption:**
  - Provide an option to decrypt and retrieve the hidden message from the stegged image.

- **Command-Line Application:**
  - All operations are performed via command-line arguments.

---

## Understanding LSB Steganography
- **Least Significant Bit (LSB) Technique:**
  - This technique modifies the least significant bits of pixel values in an image to embed secret information.
  - The changes are imperceptible to the human eye but can be decoded using specific algorithms.

- **Why BMP Format:**
  - BMP files are preferred due to their uncompressed nature, which makes bit-level manipulations easier.

- **Magic String:**
  - A pre-defined string that can help identify if an image contains hidden information.

---

## Building and Running the Project

### Compilation
To compile the project, use the following command:
```bash
gcc -o lsb_steganography lsb_steganography.c
```

### Running the Application
```bash
./lsb_steganography [options] <image_file> <text_file>
```

### Command Line Options
- `-h`: Display help screen
- `-e`: Embed the message into the image
- `-d`: Decrypt and retrieve the hidden message
- `-m`: Embed a magic string

### Sample Usage
1. Display help screen:
   ```bash
   ./lsb_steganography -h
   ```

2. Embed a message into an image:
   ```bash
   ./lsb_steganography -e image.bmp message.txt
   ```

3. Embed a message along with a magic string:
   ```bash
   ./lsb_steganography -e -m image.bmp message.txt
   ```

4. Decrypt and retrieve the hidden message:
   ```bash
   ./lsb_steganography -d image.bmp
   ```

---

## Git Repository Setup
### README Template
```markdown
# LSB Image Steganography Project

## Description
A command-line C application to hide and retrieve secret messages within BMP image files using the Least Significant Bit (LSB) technique.

## Features
- Embed and retrieve hidden messages in BMP images
- Analyze message size for compatibility
- Support for embedding a magic string
- Command-line interface for easy operation

## Compilation
```bash
gcc -o lsb_steganography lsb_steganography.c
```

## Usage
```bash
./lsb_steganography [options] <image_file> <text_file>
```

## Command Line Options
- `-h`: Display help screen
- `-e`: Embed a message into an image
- `-d`: Decrypt and retrieve a hidden message
- `-m`: Embed a magic string

## Author
KALAIVANAN D
```

This project provides practical exposure to bit-level operations and data security through steganography techniques. Happy coding!

