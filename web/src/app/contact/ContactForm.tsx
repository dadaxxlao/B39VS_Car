'use client'; // 标记为客户端组件

import React from 'react';

export default function ContactForm() {
  const handleSubmit = (e: React.FormEvent) => {
    e.preventDefault();
    alert('Demo form submitted (non-functional).');
  };

  return (
    <>
      <form onSubmit={handleSubmit}>
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
        <button type="submit" className="bg-primary text-white font-semibold py-2 px-6 rounded-md hover:bg-primary-dark transition duration-300 w-full md:w-auto">
          Send Inquiry
        </button>
      </form>
      <p className="text-xs mt-3 text-muted-foreground">
        Note: This is a demonstration website. Contact information is fictitious and form submissions are not actively monitored.
      </p>
    </>
  );
} 