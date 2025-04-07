'use client'; // 标记为客户端组件

import React from 'react';

interface VideoPlayerProps {
  videoSrc: string;
  posterSrc?: string;
}

export default function VideoPlayer({ videoSrc, posterSrc }: VideoPlayerProps) {
  return (
    <video controls className="w-full h-auto" poster={posterSrc}>
      <source src={videoSrc} type="video/mp4" />
      Your browser does not support the video tag.
    </video>
  );
} 