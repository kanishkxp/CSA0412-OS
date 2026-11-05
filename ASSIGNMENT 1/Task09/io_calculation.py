"""Task 9: interrupt-driven I/O (one interrupt per word) versus DMA for a 500 MB MRI image.
The cost of one interrupt is an assumption (2 microseconds is a typical order of magnitude)."""
image = 500 * 1024 * 1024        # bytes
word = 4                         # bytes per word
irq_cost = 2e-6                  # seconds of CPU per interrupt (assumption)
dma_block = 64 * 1024            # bytes moved per DMA transfer (one interrupt at the end of each)

irq_words = image // word
dma_irqs = image // dma_block
print(f"image size                 : {image:,} bytes")
print(f"interrupt-driven: interrupts = {irq_words:,}   CPU time = {irq_words*irq_cost:.1f} s")
print(f"DMA ({dma_block//1024} KB blocks): interrupts = {dma_irqs:,}   CPU time = {dma_irqs*irq_cost*1000:.1f} ms")
print(f"reduction in interrupts    : {irq_words/dma_irqs:,.0f} times")
print(f"buffer memory, interrupt-driven: about one word (4 B) in the device register + the final image area")
print(f"buffer memory, DMA             : one {dma_block//1024} KB block (or two for double buffering) + the final image area")
for t in (0.5, 1.0, 2.0, 5.0):
    print(f"  if one interrupt costs {t} us -> interrupt-driven CPU time = {irq_words*t*1e-6:7.1f} s")
