import java.util.LinkedList;
import java.util.Queue;

public class Experiment5 {

    // A shared buffer where items are produced and consumed
    static class SharedBuffer {
        private final Queue<Integer> queue = new LinkedList<>();
        private final int capacity;

        public SharedBuffer(int capacity) {
            this.capacity = capacity;
        }

        // Producer adds an item to the buffer
        public synchronized void produce(int value) throws InterruptedException {
            while (queue.size() == capacity) {
                System.out.println("Buffer is full! Producer is waiting...");
                wait(); // Wait until space becomes available
            }

            queue.add(value);
            System.out.println("Produced: " + value + " | Buffer size: " + queue.size());
            
            // Notify waiting consumer threads that an item is available
            notifyAll();
        }

        // Consumer removes an item from the buffer
        public synchronized int consume() throws InterruptedException {
            while (queue.isEmpty()) {
                System.out.println("Buffer is empty! Consumer is waiting...");
                wait(); // Wait until an item is produced
            }

            int value = queue.poll();
            System.out.println("Consumed ---> " + value + " | Buffer size: " + queue.size());
            
            // Notify waiting producer threads that space is available
            notifyAll();
            
            return value;
        }
    }

    // Producer Task
    static class Producer implements Runnable {
        private final SharedBuffer buffer;

        public Producer(SharedBuffer buffer) {
            this.buffer = buffer;
        }

        @Override
        public void run() {
            try {
                for (int i = 1; i <= 5; i++) {
                    buffer.produce(i);
                    Thread.sleep(500); // Simulate time taken to produce an item
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.out.println("Producer was interrupted.");
            }
        }
    }

    // Consumer Task
    static class Consumer implements Runnable {
        private final SharedBuffer buffer;

        public Consumer(SharedBuffer buffer) {
            this.buffer = buffer;
        }

        @Override
        public void run() {
            try {
                for (int i = 1; i <= 5; i++) {
                    buffer.consume();
                    Thread.sleep(800); // Simulate time taken to consume an item
                }
            } catch (InterruptedException e) {
                Thread.currentThread().interrupt();
                System.out.println("Consumer was interrupted.");
            }
        }
    }

    // Main Method to Run the Simulation
    public static void main(String[] args) {
        System.out.println("=== Multithreading Started ===");

        // Create a shared buffer with a maximum capacity of 3
        SharedBuffer buffer = new SharedBuffer(3);

        // Create Producer and Consumer threads
        Thread producerThread = new Thread(new Producer(buffer), "Producer-Thread");
        Thread consumerThread = new Thread(new Consumer(buffer), "Consumer-Thread");

        // Start the threads
        producerThread.start();
        consumerThread.start();

        try {
            // Wait for both threads to finish execution
            producerThread.join();
            consumerThread.join();
        } catch (InterruptedException e) {
            System.out.println("Main thread interrupted.");
        }

        System.out.println("=== Multithreading Completed ===");
    }
}