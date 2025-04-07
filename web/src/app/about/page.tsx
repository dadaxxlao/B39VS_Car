export default function AboutPage() {
  return (
    <div className="max-w-4xl mx-auto">
      <h1 className="text-4xl font-bold mb-6 text-center text-primary">About EcoClaw Robotics</h1>

      <section className="mb-10 p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
        <h2 className="text-2xl font-semibold mb-3 text-secondary">Our Mission</h2>
        <p className="text-lg text-muted-foreground">
          To revolutionize industrial safety and efficiency by providing advanced autonomous robotic solutions that empower the safe handling of hazardous materials and waste. We aim to protect personnel, optimize processes, and ensure environmental compliance for our partners.
        </p>
      </section>

      <section className="mb-10">
        <h2 className="text-2xl font-semibold mb-3">Who We Are</h2>
        <p className="text-lg text-muted-foreground mb-4">
          EcoClaw Robotics is a dynamic startup focused on automated systems for hazardous waste management. Founded by experts in robotics, industrial safety, and systems integration, we recognized the need for smarter, more reliable solutions to manage hazardous materials in complex industrial settings.
        </p>
        <p className="text-lg text-muted-foreground">
          While we oversee the entire lifecycle of our solutions—from design and delivery to ongoing maintenance—we employ a collaborative approach. We partner with leading third-party technology specialists to integrate cutting-edge technologies, ensuring our systems meet the highest standards of performance and reliability.
        </p>
      </section>

      <section className="mb-10 p-6 bg-gray-50 dark:bg-gray-800 rounded-lg shadow-sm">
        <h2 className="text-2xl font-semibold mb-3 text-secondary">Our Approach</h2>
        <p className="text-lg text-muted-foreground mb-4">
          Design capability and systems integration are at the heart of our engineering process. We combine creativity, technical skill, and rigorous analysis to develop robust autonomous systems tailored to specific industrial needs.
        </p>
        <p className="text-lg text-muted-foreground">
           Our current projects focus on developing autonomous robotic vehicles for industrial facility partners, addressing their unique hazardous waste challenges arising from production processes. We emphasize practical, demonstrable solutions, validated by proof-of-concepts before full deployment, ensuring our systems deliver tangible results.
        </p>
      </section>

       <section>
        <h2 className="text-2xl font-semibold mb-3">Why Choose EcoClaw?</h2>
         <ul className="list-disc list-inside text-lg text-muted-foreground space-y-2">
            <li><span className="font-semibold">Safety First:</span> Minimizing personnel risk is our top priority.</li>
            <li><span className="font-semibold">Efficiency Driven:</span> Streamlining waste handling processes to save time and resources.</li>
            <li><span className="font-semibold">Compliance Assured:</span> Helping facilities meet and exceed environmental regulations.</li>
            <li><span className="font-semibold">Innovative Technology:</span> Leveraging sensors, robotics, and intelligent navigation.</li>
             <li><span className="font-semibold">Collaborative Partnership:</span> Working closely with clients and technology experts.</li>
         </ul>
       </section>
    </div>
  );
} 