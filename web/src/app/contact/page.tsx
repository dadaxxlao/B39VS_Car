import Image from 'next/image'; // Used for map placeholder

export default function ContactPage() {
  return (
    <div className="max-w-6xl mx-auto">
      <h1 className="text-4xl font-bold mb-8 text-center text-primary">Contact EcoClaw Robotics</h1>
       <p className="text-lg text-center text-muted-foreground mb-10">
          We're ready to discuss how our autonomous solutions can meet your hazardous waste management needs. Reach out today.
       </p>

      <div className="grid md:grid-cols-2 gap-12">
        {/* Contact Information & Map */}
        <div className="space-y-6">
          <h2 className="text-2xl font-semibold mb-4 text-secondary">Get In Touch</h2>
          <div>
            <h3 className="text-lg font-medium mb-1">EcoClaw Robotics (Fictitious)</h3>
            <p className="text-muted-foreground">123 Innovation Drive</p>
            <p className="text-muted-foreground">Tech Park, CA 94000, USA</p> {/* Updated Address */}
            {/* Removed Suite number for consistency with footer */}
          </div>
           <div>
             <h3 className="text-lg font-medium mb-1">Phone</h3>
             <p className="text-muted-foreground">+1 (555) 123-4567 (Fictitious)</p> {/* Updated Phone */}
           </div>
           <div>
              <h3 className="text-lg font-medium mb-1">Email</h3>
              <p className="text-muted-foreground">
                 <a href="mailto:contact@ecoclaw-robotics.example.com" className="hover:underline">contact@ecoclaw-robotics.example.com</a> (Fictitious) {/* Updated Email */}
              </p>
           </div>

          {/* Map Placeholder */}
          <div className="mt-8">
             <h3 className="text-lg font-medium mb-2">Our Location (Conceptual)</h3>
             <div className="bg-gray-300 dark:bg-gray-700 h-64 rounded-lg flex items-center justify-center text-gray-500 dark:text-gray-400 shadow-md overflow-hidden">
               {/* Can embed iframe later, or use static image */}
               {/* Example with placeholder text: */}
                <span className="text-center">[Interactive Map Placeholder - Showing Tech Park, CA]</span>
                {/* Example with static image placeholder: */}
                {/* <Image src="/placeholder-map.png" alt="Map showing EcoClaw location" layout="fill" objectFit="cover" /> */}
             </div>
          </div>
        </div>

        {/* Contact Form */}
        <div>
          <h2 className="text-2xl font-semibold mb-4 text-secondary">Send Us a Message</h2>
          <form onSubmit={(e) => { e.preventDefault(); alert('Demo form submitted (non-functional).'); }}>
            {/* Use more descriptive labels and placeholders */}
            <div className="mb-5">
              <label htmlFor="name" className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Full Name</label>
              <input type="text" id="name" name="name" required className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-md shadow-sm focus:outline-none focus:ring-primary focus:border-primary bg-white dark:bg-gray-700" placeholder="e.g., Jane Doe" />
            </div>
            <div className="mb-5">
               <label htmlFor="company" className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Company Name</label>
               <input type="text" id="company" name="company" className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-md shadow-sm focus:outline-none focus:ring-primary focus:border-primary bg-white dark:bg-gray-700" placeholder="e.g., ABC Industries" />
            </div>
            <div className="mb-5">
              <label htmlFor="email" className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Work Email</label>
              <input type="email" id="email" name="email" required className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-md shadow-sm focus:outline-none focus:ring-primary focus:border-primary bg-white dark:bg-gray-700" placeholder="e.g., jane.doe@abcindustries.com" />
            </div>
            <div className="mb-5">
              <label htmlFor="message" className="block text-sm font-medium text-gray-700 dark:text-gray-300 mb-1">Your Inquiry</label>
              <textarea id="message" name="message" rows={5} required className="w-full px-4 py-2 border border-gray-300 dark:border-gray-600 rounded-md shadow-sm focus:outline-none focus:ring-primary focus:border-primary bg-white dark:bg-gray-700" placeholder="Please describe your needs or questions..."></textarea>
            </div>
            <button type="submit" className="bg-primary text-white font-semibold py-2 px-6 rounded-md hover:bg-primary-dark transition duration-300 w-full md:w-auto"> {/* Applied button styling */}
              Send Inquiry
            </button>
          </form>
           <p className="text-xs mt-3 text-muted-foreground">
             Note: This is a demonstration website. Contact information is fictitious and form submissions are not actively monitored.
           </p>
        </div>
      </div>
    </div>
  );
} 