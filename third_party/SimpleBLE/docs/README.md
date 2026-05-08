# SimpleBLE Documentation

This is the documentation website for [SimpleBLE](https://github.com/simpleble/simpleble), a cross-platform Bluetooth Low Energy (BLE) library.

Built with [Fumadocs](https://fumadocs.dev) and [Next.js](https://nextjs.org).

## Getting Started

Run the development server:

```bash
npm run dev
```

Open [http://localhost:3000](http://localhost:3000) with your browser to see the result.

## API Documentation Generation

The API reference is automatically generated from the C++ source code using Doxygen and a custom parser that converts Doxygen XML to MDX with React components.

### Scripts

- `npm run build:doxygen`: Orchestrates the full API documentation build.
  - Runs Doxygen using the `Doxyfile` to generate XML output in `_doxygen/xml/`.
  - Executes `scripts/build-doxygen.js` to parse the XML and generate `content/docs/simpleble/api.mdx`.
- `npm run build`: Runs the Doxygen build followed by the Next.js production build.

### Script Details

- **`scripts/build-doxygen.js`**: The main entry point for API generation. It defines which classes to include (Standard API, Safe API, etc.) and uses the helpers in `utils.js` to generate the final MDX file.
- **`scripts/utils.js`**: Contains the logic for:
  - Parsing Doxygen XML nodes.
  - Extracting text, types, and method signatures safely.
  - Converting XML structures into `<ApiClass>`, `<ApiMethod>`, and `<ApiSection>` React components.
  - Handling edge cases like C++ template types, escaping JSX/Markdown, and generating stable anchor IDs for the Table of Contents.

## Project Structure

| Route                | Description                                        |
| -------------------- | -------------------------------------------------- |
| `app/(home)`         | The landing page.                                  |
| `app/docs`           | The documentation layout and pages.                |
| `content/docs`       | MDX/Markdown files for the documentation.          |
| `src/components/api` | Custom React components used in the API reference. |
| `scripts/`           | Automation scripts for Doxygen integration.        |

## Learn More

- [Fumadocs Documentation](https://fumadocs.dev)
- [Next.js Documentation](https://nextjs.org/docs)
- [SimpleBLE Repository](https://github.com/simpleble/simpleble)
