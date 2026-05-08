'use client'

import Image from 'next/image'
import { useEffect, useState } from 'react'

type LogoWithThemeProps = {
  width: number
  height: number
  className?: string
  priority?: boolean
}

export function LogoWithTheme({
  width,
  height,
  className,
  priority = false,
}: LogoWithThemeProps): React.ReactElement {
  const [isDark, setIsDark] = useState(false)
  const [mounted, setMounted] = useState(false)

  useEffect(() => {
    setMounted(true)
    
    const checkDarkMode = () => {
      setIsDark(document.documentElement.classList.contains('dark'))
    }

    checkDarkMode()

    const observer = new MutationObserver(checkDarkMode)
    observer.observe(document.documentElement, {
      attributes: true,
      attributeFilter: ['class'],
    })

    return () => observer.disconnect()
  }, [])

  const logoSrc = mounted && isDark ? '/simpleBLE-logo.png' : '/simpleBLE-logo-dark.png'

  return (
    <Image
      src={logoSrc}
      alt="SimpleBLE"
      width={width}
      height={height}
      className={className}
      priority={priority}
    />
  )
}
