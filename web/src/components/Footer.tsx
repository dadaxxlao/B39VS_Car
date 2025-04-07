export default function Footer() {
  const currentYear = new Date().getFullYear();
  return (
    <footer className="bg-gray-200 dark:bg-gray-900 text-gray-700 dark:text-gray-300 p-6 mt-auto">
      <div className="container mx-auto text-center text-sm flex flex-col md:flex-row justify-between items-center">
        <div className="mb-4 md:mb-0">
          <p>&copy; {currentYear} EcoClaw Robotics. All rights reserved.</p>
          <p className="mt-1">
             <span className="cursor-pointer hover:underline">Privacy Policy</span> | <span className="cursor-pointer hover:underline">Terms of Service</span>
          </p>
        </div>

        <div className="text-xs text-gray-500 dark:text-gray-400">
          <p><span className="font-semibold">EcoClaw Robotics</span></p>
          <p>123 Innovation Drive, Tech Park, CA 94000</p>
          <p>Email: contact@ecoclaw-robotics.example.com</p>
          <p>Phone: +1 (555) 123-4567</p>
        </div>
      </div>
    </footer>
  );
} 