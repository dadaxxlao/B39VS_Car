import Link from 'next/link';
import Image from 'next/image'; // Keep Image import

export default function HomePage() {
  return (
    <div>
      {/* Hero Section */}
      <section className="bg-gradient-to-r from-secondary to-primary text-white py-20 mb-12 text-center">
        <div className="container mx-auto px-4">
          <h1 className="text-4xl md:text-5xl font-bold mb-4 leading-tight">Revolutionizing Hazardous Waste Management with Autonomous Robotics</h1>
          <p className="text-lg md:text-xl mb-8 max-w-3xl mx-auto">EcoClaw provides intelligent, safe, and efficient robotic systems designed for the complex demands of industrial hazardous material handling.</p>
          <Link href="/solution" className="bg-white text-secondary font-semibold py-3 px-8 rounded-full hover:bg-gray-200 transition duration-300 text-lg">
            Explore Our Solutions
          </Link>
        </div>
      </section>

      {/* Problem Statement */}
      <section className="container mx-auto px-4 py-12 mb-12">
        <h2 className="text-3xl font-bold text-center mb-6">The Urgent Need for Smarter Waste Handling</h2>
        <p className="text-center max-w-3xl mx-auto text-muted-foreground text-lg">
          Industrial facilities face increasingly complex hazardous waste management challenges. Ensuring personnel safety, optimizing disposal processes, and complying with stringent environmental regulations require innovative and reliable solutions. Traditional manual processes often introduce risks and inefficiencies.
        </p>
      </section>

      {/* Solution Introduction */}
      <section className="bg-gray-100 dark:bg-gray-800 py-16 mb-12">
        <div className="container mx-auto px-4 text-center">
          <h2 className="text-3xl font-bold mb-6">EcoClaw: Automating Safety, Ensuring Compliance</h2>
          <p className="max-w-3xl mx-auto text-muted-foreground text-lg mb-8">
            Our flagship autonomous robotic system navigates facilities using sophisticated sensor technology, following predefined paths marked by color-coded indicators. It safely identifies, grips, transports, and deposits hazardous waste containers, minimizing human intervention and enhancing operational safety and efficiency.
          </p>
           <Link href="/solution" className="text-primary dark:text-secondary font-semibold hover:underline">Learn More Technical Details</Link> {/* Adjusted styling for visibility */}
        </div>
      </section>

      {/* Key Features Overview */}
      <section className="container mx-auto px-4 py-12 mb-12">
        <h2 className="text-3xl font-bold text-center mb-10">Core System Advantages</h2>
        <div className="grid md:grid-cols-3 gap-10">
          {/* Feature 1: Navigation */}
          <div className="text-center p-6 border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm hover:shadow-md transition-shadow duration-300">
             <Image src="/3D_PCB_Docker.png" alt="Intelligent Navigation System PCB" width={80} height={80} className="mx-auto mb-4 rounded-md" /> {/* Updated src from /3D_PCB_1.png */}
            <h3 className="text-xl font-semibold mb-3">Intelligent Autonomous Navigation</h3>
            <p className="text-muted-foreground">
              Navigates complex industrial environments via line-following and color sensor data, autonomously identifying routes and specific zones (pickup, staging), capable of operating across multiple rooms/departments.
            </p>
          </div>
          {/* Feature 2: Handling */}
          <div className="text-center p-6 border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm hover:shadow-md transition-shadow duration-300">
             <Image src="/Arm.jpg" alt="Robotic Arm Gripper" width={80} height={80} className="mx-auto mb-4 rounded-md object-contain" /> {/* Updated src from /Arm.png to /Arm.jpg */}
            <h3 className="text-xl font-semibold mb-3">Secure & Reliable Handling</h3>
            <p className="text-muted-foreground">
              Equipped with a versatile gripper mechanism and secure automated storage bay. Verifies container contents and destinations via color codes before transport, ensuring material integrity.
            </p>
          </div>
          {/* Feature 3: Safety */}
          <div className="text-center p-6 border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm hover:shadow-md transition-shadow duration-300">
             <Image src="/Driver.png" alt="Safety Sensor Driver Board" width={80} height={80} className="mx-auto mb-4 rounded-md" />
            <h3 className="text-xl font-semibold mb-3">Enhanced Safety & Compliance</h3>
            <p className="text-muted-foreground">
             Significantly reduces personnel exposure to hazardous materials. Features obstacle detection (distance sensors) with audible alerts and alternate path planning capabilities, supporting adherence to safety protocols and environmental regulations.
            </p>
          </div>
        </div>
      </section>

       {/* Proof-of-Concept Teaser */}
      <section className="bg-gray-100 dark:bg-gray-800 py-16 mb-12">
         <div className="container mx-auto px-4 grid md:grid-cols-2 gap-12 items-center">
           <div>
             <h2 className="text-3xl font-bold mb-4">Validated Technology: The B39VS_Car Project</h2>
             <p className="text-muted-foreground mb-6 text-lg">
               Our core technology is validated through the successful development and demonstration of the B39VS_Car, a functional proof-of-concept prototype. This scaled model showcases key capabilities including autonomous navigation via line and color sensing, secure container pickup and placement using a gripper mechanism, and obstacle awareness within a controlled environment.
             </p>
             <Link href="/solution#poc" className="text-primary dark:text-secondary font-semibold hover:underline text-lg"> {/* Adjusted styling */}
               View Technical Details &rarr;
             </Link>
           </div>
           {/* Video Embed */}
           <div className="rounded-lg shadow-md overflow-hidden"> {/* Added container for better styling */}
             <video controls className="w-full h-auto" poster="/Arm.jpg"> {/* Updated poster from /Arm.png to /Arm.jpg */}
               <source src="/TestMovie.mp4" type="video/mp4" />
               Your browser does not support the video tag.
             </video>
           </div>
         </div>
      </section>

      {/* Innovation Highlight */}
       <section className="container mx-auto px-4 py-12 mb-12">
         <h2 className="text-3xl font-bold text-center mb-6">Our Innovative Edge</h2>
         <p className="text-center max-w-3xl mx-auto text-muted-foreground text-lg">
           EcoClaw stands out through the seamless integration of robust navigation, secure handling, and proactive safety features. Our system's ability to interpret color-coded environments and communicate status updates sets a new standard in automated hazardous waste management. We leverage accessible and reliable Arduino-based technology, making advanced automation practical.
         </p>
       </section>

      {/* Technical Showcase Section START */}
      <section className="container mx-auto px-4 py-16 mb-12">
        <h2 className="text-3xl font-bold text-center mb-10">Technical Showcase</h2>
        <div className="grid md:grid-cols-2 lg:grid-cols-3 gap-8 items-start">

          {/* PCB Design 1 */}
          <div className="text-center border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm p-4">
            <Image src="/3D_PCB_Docker.png" alt="3D PCB Design View 1" width={300} height={200} className="rounded-lg shadow-md mb-4 mx-auto object-contain" /> {/* Updated src from /3D_PCB_1.png */}
            <h3 className="text-xl font-semibold mb-2">PCB Design (Top View)</h3>
            <p className="text-muted-foreground text-sm">Detailed view of the main control board layout.</p>
          </div>

          {/* PCB Design 2 */}
          <div className="text-center border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm p-4">
            <Image src="/3D_PCB_Driver.png" alt="3D PCB Design View 2" width={300} height={200} className="rounded-lg shadow-md mb-4 mx-auto object-contain" /> {/* Updated src from /3D_PCB_2.png */}
            <h3 className="text-xl font-semibold mb-2">PCB Design (Bottom View)</h3>
            <p className="text-muted-foreground text-sm">Underside view showcasing component placement.</p>
          </div>

          {/* Driver Board */}
          <div className="text-center border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm p-4">
            <Image src="/Driver.png" alt="Soldered Driver Board" width={300} height={200} className="rounded-lg shadow-md mb-4 mx-auto object-contain" />
            <h3 className="text-xl font-semibold mb-2">Motor Driver Board</h3>
            <p className="text-muted-foreground text-sm">Close-up of the assembled motor control circuitry.</p>
          </div>

          {/* Schematic Diagram */}
          <div className="md:col-span-2 lg:col-span-3 text-center border border-gray-200 dark:border-gray-700 rounded-lg shadow-sm p-4">
             <h3 className="text-xl font-semibold mb-4">System Schematic</h3>
             <object type="image/svg+xml" data="/Sheet_Docker.svg" className="w-full h-auto max-h-[600px] rounded-lg shadow-md mx-auto"> {/* Updated data from /Sheet_Dokcer.svg */}
                 <p className="text-muted-foreground">Your browser does not support SVG objects. <a href="/Sheet_Docker.svg" className="text-primary underline">Download Schematic</a></p> {/* Updated href from /Sheet_Dokcer.svg */}
             </object>
             <p className="text-muted-foreground text-sm mt-2">Overall system electronic design.</p>
          </div>
        </div>
      </section>
      {/* Technical Showcase Section END */}


      {/* Final Call to Action (CTA) */}
      <section className="bg-secondary text-white py-16">
        <div className="container mx-auto px-4 text-center">
          <h2 className="text-3xl font-bold mb-4">Ready to Enhance Your Facility's Safety and Efficiency?</h2>
          <p className="mb-8 max-w-2xl mx-auto text-lg">
            Discover how EcoClaw's customizable autonomous solutions can address your specific hazardous waste management challenges and drive operational excellence.
          </p>
          <Link href="/contact" className="bg-white text-secondary font-semibold py-3 px-8 rounded-full hover:bg-gray-200 transition duration-300 text-lg">
            Request a Consultation
          </Link>
        </div>
      </section>
    </div>
  );
}
