import React from 'react';
// 移除未使用的导入
// import Image from 'next/image';
import ContactForm from './ContactForm';

export default function ContactPage() {
  return (
    <div className="max-w-6xl mx-auto">
      <h1 className="text-4xl font-bold mb-8 text-center text-primary">Contact EcoClaw Robotics</h1>
       <p className="text-lg text-center text-muted-foreground mb-10">
          We&apos;re ready to discuss how our autonomous solutions can meet your hazardous waste management needs. Reach out today.
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

        {/* Contact Form - 使用客户端组件 */}
        <div>
          <h2 className="text-2xl font-semibold mb-4 text-secondary">Send Us a Message</h2>
          <ContactForm />
        </div>
      </div>
    </div>
  );
} 