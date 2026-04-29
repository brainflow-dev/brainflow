import defaultMdxComponents from 'fumadocs-ui/mdx';
import type { MDXComponents } from 'mdx/types';

export function getMDXComponents(components?: MDXComponents): MDXComponents {
  return {
    ...defaultMdxComponents,
    h1: (props) => (
      <h1 {...props} className="font-sora font-bold tracking-tight text-3xl mt-12 mb-6 border-b pb-2 border-fd-border/50" />
    ),
    h2: (props) => (
      <h2 {...props} className="font-sora font-semibold tracking-tight text-2xl mt-10 mb-4" />
    ),
    h3: (props) => (
      <h3 {...props} className="font-sora font-semibold tracking-tight text-xl mt-8 mb-3" />
    ),
    blockquote: (props) => (
      <blockquote {...props} className="border-l-4 border-primary bg-primary/5 px-6 py-4 my-8 rounded-r-2xl italic text-fd-muted-foreground" />
    ),
    ...components,
  };
}
