import random

def generate_number_file(filename, num_lines=120):
    with open(filename, 'w') as file:
        for _ in range(num_lines):
            # Generate a list of numbers from 1 to num_lines
            numbers = list(range(1, num_lines + 1))
            # Shuffle the list to create a random order
            random.shuffle(numbers)
            # Join the numbers with commas and write to the file
            line = ",".join(str(num) for num in numbers)
            file.write(line + "\n")

# Call the function to generate the file
generate_number_file("seed.txt")
