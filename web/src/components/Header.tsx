import Link from 'next/link';
import Image from 'next/image';

export default function Header() {
  return (
    <header className="bg-gray-800 text-white p-4 shadow-md sticky top-0 z-50">
      <nav className="container mx-auto flex justify-between items-center">
        <Link href="/" className="flex items-center space-x-2 hover:opacity-80 transition duration-300">
          <Image src="/logo.png" alt="EnviroSecure Logo" width={150} height={40} priority />
        </Link>
        <ul className="flex space-x-4 md:space-x-6">
          <li><Link href="/" className="hover:text-cyan-300 transition duration-200">Home</Link></li>
          <li><Link href="/about" className="hover:text-cyan-300 transition duration-200">About Us</Link></li>
          <li><Link href="/solution" className="hover:text-cyan-300 transition duration-200">Our Solution</Link></li>
          <li><Link href="/contact" className="hover:text-cyan-300 transition duration-200">Contact Us</Link></li>
        </ul>
      </nav>
    </header>
  );
} 