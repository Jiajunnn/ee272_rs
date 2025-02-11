import pandas as pd
import matplotlib.pyplot as plt
import sys

def plot_histograms(csv_file):
    data = pd.read_csv(csv_file)
    
    bin_size = 10
    max_lifetime = data['Lifetime'].max()
    bins = range(0, max_lifetime + bin_size, bin_size)
    
    num_buffers = data['Buffer'].nunique()
    fig, axes = plt.subplots(nrows=num_buffers, ncols=1, figsize=(10, 5 * num_buffers))
    fig.subplots_adjust(hspace=0.5)
    
    for buffer, ax in zip(data['Buffer'].unique(), axes.flatten()):
        buffer_data = data[data['Buffer'] == buffer]
        ax.hist(buffer_data['Lifetime'], bins=bins, alpha=0.7, edgecolor='black')
        ax.set_title(f'Lifetime Histogram for Buffer {buffer}')
        ax.set_xlabel(f'Lifetime (Grouped by {bin_size}ns)')
        ax.set_ylabel('Frequency')
    
    output_file = 'histograms_buffer.png'
    plt.savefig(output_file)
    plt.close()
    
    print(f"Histograms grouped by {bin_size}ns saved to {output_file}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python script.py <csv_file>")
        sys.exit(1)
    plot_histograms(sys.argv[1])
