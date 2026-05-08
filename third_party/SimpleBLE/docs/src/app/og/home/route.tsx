import { ImageResponse } from "next/og";

import {
  generate as MetadataImage,
  getImageResponseOptions,
} from "@/components/ui/og-generate";

export const revalidate = false;

export async function GET(_req: Request) {
  const title = "SimpleBLE Docs";
  const description =
    "Learn the core API, pick the right bindings, and follow practical guides for building reliable Bluetooth experiences across platforms.";

  return new ImageResponse(
    <MetadataImage title={title} description={description} />,
    await getImageResponseOptions(),
  );
}

export function generateStaticParams() {
  return [];
}
