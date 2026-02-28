import fs from 'fs';
import path from 'path';

const appAsarRegexp = new RegExp(`\\${path.sep}app\\.asar\\${path.sep}`, 'g');

export function resolveLibPath(): string
{
    const defaultLibPath = path.resolve(__dirname, '..', 'brainflow', 'lib');
    const electronProcess = process as NodeJS.Process & {resourcesPath?: string};
    const searchRoots = [
        __dirname,
        __dirname.replace(appAsarRegexp, `${path.sep}app.asar.unpacked${path.sep}`),
        typeof electronProcess.resourcesPath === 'string' ? electronProcess.resourcesPath : '',
    ].filter(Boolean);

    const libPathCandidates = [
        defaultLibPath,
        ...searchRoots.flatMap((root) => [
            path.resolve(root, '..', 'brainflow', 'lib'),
            path.resolve(root, '..', 'node_modules', 'brainflow', 'brainflow', 'lib'),
            path.resolve(root, 'node_modules', 'brainflow', 'brainflow', 'lib'),
            path.resolve(root, 'app.asar.unpacked', 'node_modules', 'brainflow', 'brainflow', 'lib'),
        ]),
    ];

    const existingPath = libPathCandidates.find((candidate) => fs.existsSync(candidate));
    return existingPath || defaultLibPath;
}
