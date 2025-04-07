import Link from 'next/link';
import Image from 'next/image'; // Optional, for visual placeholders

export default function SolutionPage() {
  return (
    <div className="max-w-5xl mx-auto">
      <h1 className="text-4xl font-bold mb-6 text-center text-primary">EcoClaw Autonomous Waste Management Robot</h1>

      <section className="mb-10">
        <p className="text-xl text-center text-muted-foreground mb-8">
          Our core offering is a sophisticated, Arduino-based autonomous robotic vehicle designed for the safe and efficient handling, transport, and delivery of hazardous waste containers within industrial indoor environments.
        </p>
         {/* Robot Main Image/Diagram Placeholder */}
         <div className="bg-gray-300 dark:bg-gray-700 h-80 flex items-center justify-center text-gray-500 dark:text-gray-400 rounded-lg shadow-md mb-10">
             {/* Replace with actual Image component later */}
             <Image src="/Arm.jpg" alt="EcoClaw Robot Concept" width={300} height={300} className="object-contain" /> {/* Updated src from /Arm.png to /Arm.jpg */}
         </div>
      </section>

      <h2 className="text-3xl font-semibold mt-10 mb-6 text-secondary">Core Capabilities & Features</h2>

      <div className="space-y-8">
        {/* Feature Section 1 */}
        <section className="p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
          <h3 className="text-2xl font-semibold mb-3">Autonomous Navigation & Path Following</h3>
          <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
            <li>Capable of autonomous navigation through a specified number of rooms, departments, or zones within a facility.</li>
            <li>Utilizes line identification and following sensors for precise tracking of predefined routes.</li>
            <li>Employs color sensors to identify specific destinations, staging areas, and potentially container types based on color-coded markings and containers.</li>
             <li>Designed to operate reliably without continuous operator intervention once a task is initiated.</li>
          </ul>
        </section>

        {/* Feature Section 2 */}
         <section className="p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
          <h3 className="text-2xl font-semibold mb-3">Secure Container Handling & Transport</h3>
          <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
            <li>Integrated gripper system capable of securely picking up various hazardous material containers.</li>
            <li>Features a secure bay that locks automatically once container placement is detected, maintaining security during transport.</li>
            <li>Follows a defined workflow: navigate to pickup point, locate container, verify with color sensor (optional step based on scenario), pickup container, secure bay, transport to base, unload into appropriate staging space.</li>
            <li>Handles operations involving multiple pickup points, returning containers to a primary base/staging area.</li>
          </ul>
        </section>

        {/* Feature Section 3 */}
         <section className="p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
           <h3 className="text-2xl font-semibold mb-3">Advanced Safety & Obstacle Management</h3>
           <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
             <li>Uses distance sensors (e.g., ultrasonic, infrared) to detect personnel or objects obstructing its path.</li>
             <li>Initiates audible commands or signals to notify individuals to clear the way.</li>
             <li>If no response is detected or the obstacle persists, the robot is designed to attempt finding an alternative route (requires appropriate path planning logic).</li>
             <li>Prioritizes safe operation, minimizing risks to personnel and equipment.</li>
           </ul>
         </section>

         {/* Feature Section 4 */}
         <section className="p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
           <h3 className="text-2xl font-semibold mb-3">System Integration & Communication</h3>
           <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
              <li>Core control based on the reliable Arduino microcontroller platform.</li>
              <li>Includes user interface (details TBD - could be simple buttons/web interface) and clear LED indicators for status updates (e.g., navigating, carrying load, charging, error).</li>
              <li><span className="font-semibold text-accent">[Advanced Feature]</span> Capable of communicating its operational status (e.g., location, task progress, battery level) back to a main computer system for monitoring.</li>
           </ul>
         </section>

         {/* Feature Section 5 */}
         <section className="p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
            <h3 className="text-2xl font-semibold mb-3">Hardware & Design Specifications</h3>
            <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
               <li>Utilizes at least two motors (up to six allowed) for robust mobility.</li>
               <li>Integrates necessary sensors: one color sensor, multiple distance sensors.</li>
               <li>Can incorporate additional peripherals like IR sensors or limit switches as needed for specific functions.</li>
               <li>Compact design, aiming for a 30cm x 30cm x 30cm volume.</li>
               <li>Includes a charging solution designed for the robot's base, enabling autonomous charging cycles.</li>
            </ul>
         </section>
      </div>

      <section id="poc" className="mt-12 pt-8 border-t border-gray-300 dark:border-gray-600">
        <h2 className="text-3xl font-semibold mb-6 text-secondary">Proof of Concept: The B39VS_Car Project</h2>
         <div className="grid md:grid-cols-2 gap-8 items-start">
            <div>
                <p className="text-lg text-muted-foreground mb-4">
                To validate our design and showcase core functionality, we developed the B39VS_Car prototype. This fully operational, scaled-down system served as a proof-of-concept, demonstrating the integration of navigation, perception, and handling mechanisms.
                </p>
                 <p className="text-lg text-muted-foreground mb-4">Key features demonstrated include:</p>
                 <ul className="list-disc list-inside text-lg text-muted-foreground space-y-1 mb-4">
                    <li>Line following for path navigation.</li>
                    <li>Color detection for zone/container identification.</li>
                     <li>Obstacle detection and basic avoidance/alerting.</li>
                    <li>Gripper mechanism for container interaction.</li>
                    <li>Arduino-based control logic.</li>
                 </ul>
                <p className="text-lg text-muted-foreground">
                This PoC provided tangible evidence of our system&apos;s feasibility and served as the foundation for the full-scale industrial solution. It allowed for iterative testing and refinement in a controlled setting.
                </p>
            </div>
             {/* PoC Visual Placeholders */}
             <div className="space-y-4">
                <div className="bg-gray-300 dark:bg-gray-700 h-64 flex items-center justify-center text-gray-500 dark:text-gray-400 rounded-lg shadow-md overflow-hidden">
                   {/* Replace with actual PoC image */}
                   <Image src="/3D_PCB_Docker.png" alt="B39VS_Car Prototype PCB" width={250} height={250} className="object-contain" /> {/* Updated src from /3D_PCB_1.png to /3D_PCB_Docker.png */}
                </div>
                 <div className="bg-gray-300 dark:bg-gray-700 h-48 flex items-center justify-center text-gray-500 dark:text-gray-400 rounded-lg shadow-md overflow-hidden">
                    {/* Replace with actual PoC video or gif */}
                    <span className="text-center">[Visual: Short video/gif showing PoC navigation or gripping]</span>
                 </div>
             </div>
         </div>
      </section>

      <section className="mt-12 text-center py-10 bg-gradient-to-r from-secondary to-primary text-white rounded-lg">
          <h2 className="text-3xl font-bold mb-4">Innovating for a Safer Tomorrow</h2>
          <p className="max-w-3xl mx-auto text-lg mb-6">
              EcoClaw is committed to pushing the boundaries of autonomous technology to create safer, more efficient, and compliant industrial workplaces.
          </p>
          <Link href="/contact" className="bg-white text-secondary font-semibold py-3 px-8 rounded-full hover:bg-gray-200 transition duration-300 text-lg">
              Discuss Your Needs
          </Link>
      </section>
    </div>
  );
} 