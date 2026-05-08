import { source } from "@/lib/source";
import { notFound } from "next/navigation";
import { ImageResponse } from "next/og";

import {
  generate as MetadataImage,
  getImageResponseOptions,
} from "@/components/ui/og-generate";
import { getPageImage } from "@/lib/metadata";

export const revalidate = false;

export async function GET(
  _req: Request,
  { params }: { params: Promise<{ slug: string[] }> },
) {
  const { slug } = await params;
  const page = source.getPage(slug.slice(0, -1));
  if (!page) notFound();

  return new ImageResponse(
    <MetadataImage
      title={page.data.title}
      description={page.data.description}
    />,
    await getImageResponseOptions(),
  );
}

// Pre-generate images at build time
export function generateStaticParams() {
  return source.getPages().map((page) => ({
    slug: getPageImage(page).segments,
  }));
}
