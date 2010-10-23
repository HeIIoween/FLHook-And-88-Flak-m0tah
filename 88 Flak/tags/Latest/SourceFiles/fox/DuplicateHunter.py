import string

def main():
    # Print that stuff!
    print "Quick hack of a Python homework I did once to weed out duplicates."

    # Open that file!
    try:
        inputfile = raw_input("Please enter the file to open (e.g. words.txt): ")
        infile = open(inputfile, 'r')
    except:
        print "File is not valid! File must be a sorted list of words, with one word per line."
        raw_input("Press <Enter> to quit.")
        return 1

    # Read that data!
    print "Non-duplicates:"
    prevline = "blank"
    for line in infile:
        if (prevline != line):
            print line
        prevline = line

    # Close that file!
    infile.close()
    raw_input("Press <Enter> to quit.")
    return 0

main()
