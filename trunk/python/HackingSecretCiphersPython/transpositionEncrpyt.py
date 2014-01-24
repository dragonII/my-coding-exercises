import pyperclip

def main():
    myMessage = 'Common sense is not s common.'
    myKey = 8

    ciphertext = encryptMessage(myKey, myMessage)

    # Print the encrpyted string in ciphertext to the screen, with
    # a | (called "pipe" character) after it in case there are spaces at
    # the end of the encrpyted message.
    print(ciphertext + '|')

    # Copy the encrpyted text to the clipboard.
    pyperclip.copy(ciphertext)

def encryptMessage(key, message):
    # Each string in ciphertext represents a column in the grid.
    ciphertext = [''] * key

    # Loop through each colunm in ciphertext
    for col in range(key):
        pointer = col

        # Keep looping until pointer goes past the length of the message.
        while pointer < len(message):
            # Place the character at pointer in message at the end of the
            # current column in the ciphertext list.
            ciphertext[col] += message[pointer]

            # move pointer over
            pointer += key

        # Covert the ciphertext list into a single string value and return it.
        return ''.join(ciphertext)

# If transpositionEncrypt.py is run (instead of imported as a module) call
# the main() function.
if __name__ == '__main__':
    main()
