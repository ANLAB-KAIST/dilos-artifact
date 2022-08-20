import java.lang.management.ManagementFactory;

import org.graalvm.nativeimage.*;
import org.graalvm.nativeimage.Isolates.CreateIsolateParameters;

public class Hello {
    public static void main(String[] args) {
        System.out.println("Hello, World from GraalVM on OSv!");

        /* Create a new isolate for the next function evaluation. */
        IsolateThread newContext = Isolates.createIsolate(CreateIsolateParameters.getDefault());

        System.out.println("Hello, World from GraalVM on OSv from an isolate!");

        /* Tear down the isolate, freeing all the temporary objects. */
        Isolates.tearDownIsolate(newContext);

        long currentMemory = ManagementFactory.getMemoryMXBean().getHeapMemoryUsage().getUsed();
        System.out.println("Memory usage: " + currentMemory / 1024 + " KByte" );
    }
}
