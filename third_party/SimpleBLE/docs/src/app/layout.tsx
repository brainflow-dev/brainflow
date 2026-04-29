import type { Metadata } from 'next';
import { Geist, Geist_Mono, Sora } from 'next/font/google';
import { RootProvider } from 'fumadocs-ui/provider/next';
import type { ReactNode } from 'react';
import Script from 'next/script';

import './global.css';

const geistSans = Geist({
  variable: '--font-geist-sans',
  subsets: ['latin'],
});

const geistMono = Geist_Mono({
  variable: '--font-geist-mono',
  subsets: ['latin'],
});

const sora = Sora({
  variable: '--font-sora',
  subsets: ['latin'],
  weight: ['400', '500', '600', '700'],
});

const siteUrl =
  process.env.NEXT_PUBLIC_SITE_URL ?? 'https://docs.simpleble.org';

export const metadata: Metadata = {
  metadataBase: new URL(siteUrl),
  title: 'SimpleBLE Docs',
  description: 'The fully supported, cross-platform library designed to get your embedded product to market without the headaches.',
  icons: {
    icon: '/favicon.png',
    apple: '/favicon.png',
  },
  openGraph: {
    type: 'website',
    url: siteUrl,
    siteName: 'SimpleBLE Docs',
    title: 'SimpleBLE Docs',
    description: 'The fully supported, cross-platform library designed to get your embedded product to market without the headaches.',
    images: [
      {
        url: '/og/home',
        width: 1200,
        height: 630,
        alt: 'SimpleBLE Docs',
      },
    ],
  },
  twitter: {
    card: 'summary_large_image',
    title: 'SimpleBLE Docs',
    description: 'The fully supported, cross-platform library designed to get your embedded product to market without the headaches.',
    images: ['/og/home'],
  },
};

export default function RootLayout({ children }: { children: ReactNode }) {
  return (
    <html lang="en" suppressHydrationWarning>
      <head>
        {/* Google Analytics */}
        <Script
          src="https://www.googletagmanager.com/gtag/js?id=AW-16819593911"
          strategy="afterInteractive"
        />
        <Script id="google-analytics" strategy="afterInteractive">
          {`
            window.dataLayer = window.dataLayer || [];
            function gtag(){dataLayer.push(arguments);}
            gtag('js', new Date());
            gtag('config', 'AW-16819593911');
          `}
        </Script>

        {/* Plausible Analytics */}
        <Script
          src="https://plausible.io/js/pa-COmM4LSGsz8Def-zvuyEx.js"
          strategy="afterInteractive"
        />
        <Script id="plausible-analytics" strategy="afterInteractive">
          {`
            window.plausible=window.plausible||function(){(plausible.q=plausible.q||[]).push(arguments)},plausible.init=plausible.init||function(i){plausible.o=i||{}};
            plausible.init()
          `}
        </Script>
      </head>
      <body
        className={`${geistSans.variable} ${geistMono.variable} ${sora.variable} antialiased`}
      >
        <RootProvider>{children}</RootProvider>
      </body>
    </html>
  );
}
